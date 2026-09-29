#include <chrono>
#include <opencv2/opencv.hpp>

#include "fmt/core.h"
#include "io/camera.hpp"
#include "tasks/yolo.hpp"
#include "tools/img_tools.hpp"


// 相机内参
static const cv::Mat camera_matrix =
    (cv::Mat_<double>(3,3) <<
    1286.307063384126, 0, 645.34450819155256,
    0,1288.1400736562441,483.6163720308021,
    0,0,1);


// 畸变参数
static const cv::Mat distort_coeffs =
    (cv::Mat_<double>(1,5) <<
    -0.47562935060124745,
    0.21831745829617311,
    0.0004957613589406044,
    -0.00034617769548693592,
    0);


static const double LIGHTBAR_LENGTH = 0.056;
static const double ARMOR_WIDTH = 0.135;


// Task01
static const std::vector<cv::Point3f> object_points
{
    {-ARMOR_WIDTH/2,  ARMOR_WIDTH/2, 0},   //左上
    { ARMOR_WIDTH/2,  ARMOR_WIDTH/2, 0},   //右上
    { ARMOR_WIDTH/2, -ARMOR_WIDTH/2, 0},   //右下
    {-ARMOR_WIDTH/2, -ARMOR_WIDTH/2, 0}    //左下
};



int main(int argc,char* argv[])
{

    auto_aim::YOLO detector("configs/yolo.yaml");

    io::Camera camera("configs/yolo.yaml");


    cv::Mat img;

    std::chrono::steady_clock::time_point timestamp;



    while(true)
    {

        camera.read(img,timestamp);


        if(img.empty())
            break;



        auto armors = detector.detect(img);



        if(!armors.empty())
        {


            auto armor = armors.front();



            tools::draw_points(img,armor.points);



            for(int i=0;i<4;i++)
            {
                tools::draw_text(
                    img,
                    std::to_string(i),
                    armor.points[i],
                    cv::Scalar(255,0,255),
                    1.5,
                    3
                );
            }



            fmt::print(
            "0:({:.0f},{:.0f}) "
            "1:({:.0f},{:.0f}) "
            "2:({:.0f},{:.0f}) "
            "3:({:.0f},{:.0f})\n",

            armor.points[0].x,
            armor.points[0].y,

            armor.points[1].x,
            armor.points[1].y,

            armor.points[2].x,
            armor.points[2].y,

            armor.points[3].x,
            armor.points[3].y
            );




            //==============================
            // Task02
            //==============================

            std::vector<cv::Point2f> img_points
            {
                armor.points[0],
                armor.points[1],
                armor.points[2],
                armor.points[3]
            };





            //==============================
            // Task03
            //==============================


            cv::Mat rvec,tvec;


            cv::solvePnP(
                object_points,
                img_points,
                camera_matrix,
                distort_coeffs,
                rvec,
                tvec
            );






            //==============================
            // Task04
            //==============================


            tools::draw_text(
                img,
                fmt::format(
                "tvec: x{: .2f} y{: .2f} z{: .2f}",
                tvec.at<double>(0),
                tvec.at<double>(1),
                tvec.at<double>(2)
                ),
                cv::Point(10,60),
                cv::Scalar(0,255,255),
                1.7,
                3
            );



            tools::draw_text(
                img,
                fmt::format(
                "rvec: x{: .2f} y{: .2f} z{: .2f}",
                rvec.at<double>(0),
                rvec.at<double>(1),
                rvec.at<double>(2)
                ),
                cv::Point(10,120),
                cv::Scalar(0,255,255),
                1.7,
                3
            );







            //==============================
            // Task05
            // rvec -> rmat -> Euler
            //==============================



            cv::Mat rmat;


            cv::Rodrigues(
                rvec,
                rmat
            );



            double yaw =
                atan2(
                    rmat.at<double>(1,0),
                    rmat.at<double>(0,0)
                );



            double pitch =
                atan2(
                    -rmat.at<double>(2,0),
                    sqrt(
                        rmat.at<double>(0,0)*
                        rmat.at<double>(0,0)
                        +
                        rmat.at<double>(1,0)*
                        rmat.at<double>(1,0)
                    )
                );



            double roll =
                atan2(
                    rmat.at<double>(2,1),
                    rmat.at<double>(2,2)
                );




            yaw *= 180/CV_PI;
            pitch *= 180/CV_PI;
            roll *= 180/CV_PI;





            tools::draw_text(
                img,
                fmt::format(
                "euler angles: yaw{: .2f} pitch{: .2f} roll{: .2f}",
                yaw,
                pitch,
                roll
                ),
                cv::Point(10,180),
                cv::Scalar(0,255,255),
                1.7,
                3
            );



        }




        cv::imshow(
            "press q to quit, space to pause",
            img
        );



        int key=cv::waitKey(20);


        if(key=='q')
            break;


        if(key==' ')
            cv::waitKey(0);


    }



    cv::destroyAllWindows();


    return 0;

}