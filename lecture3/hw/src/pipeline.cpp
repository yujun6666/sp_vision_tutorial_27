#include "pipeline.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <opencv2/imgcodecs.hpp>

namespace
{
    std::mutex output_mutex;

    void logLine(std::ostream &output, const std::string &message)
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        output << message << '\n';
    }
}

Pipeline::Pipeline(
    std::unique_ptr<FrameSource> source,
    PipelineConfig config)
    : source_(std::move(source)),
      config_(std::move(config))
{
    if (!source_)
    {
        throw std::invalid_argument("Pipeline requires a frame source");
    }

    if (config_.worker_count < 2)
    {
        throw std::invalid_argument("worker_count must be at least 2");
    }
}

Pipeline::~Pipeline()
{
    // 析构时回收全部线程，不重新抛出后台错误。
    joinThreads();
}

void Pipeline::start()
{
    if (started_)
    {
        throw std::logic_error("Pipeline can only be started once");
    }

    std::filesystem::create_directories(config_.output_directory);
    workers_.reserve(static_cast<std::size_t>(config_.worker_count));
    started_ = true;

    try
    {
        for (int i = 0; i < config_.worker_count; ++i)
        {
            workers_.emplace_back([this, i]
            {
                try
                {
                    workerLoop(i);
                }
                catch (...)
                {
                    recordError(std::current_exception());
                }
            });
        }

        producer_ = std::thread([this]
        {
            producerLoop();
        });
    }
    catch (...)
    {
        // 启动中途失败，也要唤醒并回收已经创建的 worker。
        queue_.close();
        joinThreads();
        throw;
    }
}

void Pipeline::wait()
{
    joinThreads();

    std::exception_ptr error;
    {
        std::lock_guard<std::mutex> lock(error_mutex_);
        error = error_;
    }

    // 所有线程回收完毕后，再向调用者报告后台错误。
    if (error)
    {
        std::rethrow_exception(error);
    }
}

void Pipeline::joinThreads()
{
    if (!started_)
    {
        return;
    }

    if (producer_.joinable())
    {
        producer_.join();
    }

    // 等生产结束再关闭队列，避免丢掉尚未入队的帧。
    // 队列关闭后，worker 仍然会取完剩余元素。
    queue_.close();

    for (auto &worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void Pipeline::recordError(std::exception_ptr error)
{
    std::lock_guard<std::mutex> lock(error_mutex_);

    if (!error_)
    {
        error_ = error;
    }
}

StatisticsSnapshot Pipeline::statistics() const
{
    return statistics_.snapshot();
}

void Pipeline::producerLoop()
{
    try
    {
        Frame frame;

        while (source_->next(frame))
        {
            statistics_.onProduced();

            logLine(
                std::cout,
                "[Producer] frame " + std::to_string(frame.id));

            // 将当前帧转交给队列。
            queue_.push(std::move(frame));
        }
    }
    catch (...)
    {
        recordError(std::current_exception());
    }

    // 正常结束或读取异常，都必须唤醒等待中的 worker。
    queue_.close();
}

void Pipeline::workerLoop(int worker_id)
{
    Frame frame;

    while (queue_.pop(frame))
    {
        if (config_.worker_delay_ms > 0)
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(config_.worker_delay_ms));
        }

        if (checksum(frame.image) != frame.expected_checksum)
        {
            statistics_.onCorrupted();

            logLine(
                std::cerr,
                "[Worker " + std::to_string(worker_id) +
                    "] ERROR: frame " + std::to_string(frame.id) +
                    " data changed before processing");

            continue;
        }

        logLine(
            std::cout,
            "[Worker " + std::to_string(worker_id) +
                "] processing frame " + std::to_string(frame.id));

        const cv::Mat output = processor_.process(frame);
        statistics_.onProcessed();

        std::ostringstream filename;
        filename << std::setw(3) << std::setfill('0')
                 << frame.id << ".jpg";

        const auto name = filename.str();
        const auto output_path =
            (config_.output_directory / name).string();

        if (cv::imwrite(output_path, output))
        {
            statistics_.onSaved();
        }
        else
        {
            throw std::runtime_error("failed to save frame: " + name);
        }
    }
}