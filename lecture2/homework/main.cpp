#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <opencv2/opencv.hpp>

#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "tools/img_tools.hpp"


int main()
{
  try {

    const std::string config_path = "./configs/yolo.yaml";


    // 初始化YOLO
    auto_aim::YOLO yolo(config_path, false);


    // 初始化相机
    io::Camera camera(config_path);


    cv::namedWindow("Armor detection", cv::WINDOW_NORMAL);


    int frame_count = 0;
    int empty_frames = 0;


    while (true)
    {

      cv::Mat img = camera.read();


      if (!img.empty())
      {

        empty_frames = 0;


        // YOLO检测
        const auto armors = yolo.detect(img, frame_count++);



        for (const auto & armor : armors)
        {

          // ============================
          // 1. 画绿色装甲板框
          // ============================

          tools::draw_points(
              img,
              armor.points,
              cv::Scalar(0,255,0),
              2
          );



          // ============================
          // 2. 获取装甲板名字
          // ============================

          std::string name;


          if (armor.name >= auto_aim::one &&
              armor.name <= auto_aim::not_armor)
          {
            name = auto_aim::ARMOR_NAMES[armor.name];
          }
          else
          {
            name = "unknown";
          }



          // ============================
          // 3. 获取颜色
          // ============================

          std::string color;


          if (armor.color >= auto_aim::red &&
              armor.color <= auto_aim::purple)
          {
            color = auto_aim::COLORS[armor.color];
          }
          else
          {
            color = "unknown";
          }



          std::string text = color + name;



          // ============================
          // 4. 显示文字位置
          // ============================

          cv::Point text_pos(
              static_cast<int>(armor.points[0].x),
              static_cast<int>(armor.points[0].y) - 10
          );



          // 防止文字跑出屏幕
          if(text_pos.y < 30)
          {
            text_pos.y = 30;
          }



          // ============================
          // 5. 绘制文字
          // ============================

          cv::putText(
              img,
              text,
              text_pos,
              cv::FONT_HERSHEY_SIMPLEX,
              1.0,
              cv::Scalar(0,0,255),
              2
          );



          // 显示置信度

          std::string conf =
              "conf:" +
              std::to_string(armor.confidence).substr(0,4);


          cv::putText(
              img,
              conf,
              text_pos + cv::Point(0,30),
              cv::FONT_HERSHEY_SIMPLEX,
              0.7,
              cv::Scalar(255,0,0),
              2
          );

        }



        cv::imshow("Armor detection", img);

      }
      else
      {

        empty_frames++;

        if(empty_frames >= 5)
        {
          throw std::runtime_error(
              "Camera returned no image"
          );
        }

      }



      int key=cv::waitKey(1);


      if(key=='q' || key=='Q' || key==27)
      {
        break;
      }


    }


    cv::destroyAllWindows();

    return 0;


  }
  catch(const std::exception & e)
  {

    std::cerr
        <<"Armor detection failed: "
        <<e.what()
        <<std::endl;

    return 1;

  }

}