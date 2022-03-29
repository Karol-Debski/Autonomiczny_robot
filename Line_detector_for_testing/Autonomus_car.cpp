#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>

using namespace std;
using namespace cv;

cv::Mat test;

void showImage(cv::Mat& frame) {
    cv::imshow("Widok", frame);
    waitKey(0);
}
void convertToGray(cv::Mat*& frame) {
    cvtColor(*frame, *frame, COLOR_BGR2GRAY);
}
void smoothImage(cv::Mat*& frame) {
    cv::GaussianBlur(*frame, *frame, cv::Size(5, 5), 0);
}
void detectEdges(cv::Mat*& frame) {

    cv::Canny(*frame, *frame, 250, 255);
}
void regionOfInterest(cv::Mat*& frame) {
    int width = frame->size[1];
    int height = frame->size[0];
    cv::Mat mask = cv::Mat::zeros(height, width, frame->type());
    cv::Point rect[1][4];
    rect[0][0] = cv::Point2i(0, height);
    rect[0][1] = cv::Point2i(0, height / 2);
    rect[0][2] = cv::Point2i(width, height / 2);
    rect[0][3] = cv::Point2i(width, height);
    const cv::Point* polygon[1] = { rect[0] };
    cv::fillConvexPoly(mask, *polygon, 4, cv::Scalar(255));
    bitwise_and(*frame, mask, *frame);
}
vector<double> slope_and_y_intersects(vector<Vec4i>& lines, cv::Mat*& frame) {
    double slope_on_left = 0.0;
    double slope_on_right = 0.0;
    double d_y = 0.0;
    double d_x = 0.0;
    int numbers_of_slope_on_left = 0;
    int numbers_of_slope_on_right = 0;
    double x_left = 0;
    double x_right = 0;
    double y_left = 0;
    double y_right = 0;
    for (int i = 0; i < lines.size(); i++) {
        int x1 = lines[i][0];
        int y1 = lines[i][1];
        int x2 = lines[i][2];
        int y2 = lines[i][3];
        d_y = (y2 - y1);
        d_x = abs(x2 - x1);
        //cout<<y1<<" "<<y2<<endl;
        if (d_y < 0 & x1 < 350 & x2 < 350 & x1>0 & x2>0) {
            numbers_of_slope_on_left++;
            slope_on_left += d_y / d_x;
            x_left += x1 + x2;
            y_left += y1 + y2;
            //cout << x1 << " " << x2 << endl;
        }
        if (d_y > 0 & x1 > 370 & x2 > 370 & x1 < 720 & x2 < 720) {
            numbers_of_slope_on_right++;
            slope_on_right += d_y / d_x;
            x_right += x1 + x2;
            y_right += y1 + y2;
        }
    }
    double average_x_left = x_left / (2 * numbers_of_slope_on_left);
    double average_x_right = x_right / (2 * numbers_of_slope_on_right);
    double average_y_left = y_left / (2 * numbers_of_slope_on_left);
    double average_y_right = y_right / (2 * numbers_of_slope_on_right);

    slope_on_left = -slope_on_left / numbers_of_slope_on_left;
    slope_on_right = slope_on_right / numbers_of_slope_on_right;

    double height = frame->size[0];
    cv::Point first_point_on_left(average_x_left - ((height - average_y_left) / slope_on_left), height);
    cv::Point second_point_on_left(average_x_left + ((height - average_y_left) / slope_on_left), height / 2);
    line(test, first_point_on_left, second_point_on_left, Scalar(0, 0, 255), 20);

    cv::Point first_point_on_right(average_x_right + ((height - average_y_right) / slope_on_right), height);
    cv::Point second_point_on_right(average_x_right - ((height - average_y_right) / slope_on_right), height / 2);
    line(test, first_point_on_right, second_point_on_right, Scalar(0, 255, 0), 20);
    vector<double> average_x;
    average_x = { average_x_left,average_x_right };
    return average_x;
}
std::vector<Vec4i> HoughTransform(cv::Mat*& frame) {
    std::vector<Vec4i> lines;
    HoughLinesP(*frame, lines, 2, CV_PI / 180, 20, 10, 100);
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        line(test, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(255, 0, 0), 10);
    }
    return lines;
}

int main(int argc, char** argv)
{
    cv::namedWindow("video", cv::WINDOW_AUTOSIZE);
    cv::VideoCapture cap;
    cap.open("raw_road.mp4");

    cv::Mat* frame = new cv::Mat;
    for (;;) {
        cap >> *frame;
        if (frame->empty()) { break; }
        test = *frame;
        cvtColor(*frame, *frame, COLOR_BGR2HSV);
        cv::GaussianBlur(*frame, *frame, cv::Size(5, 5), 0);
        cv::inRange(*frame, Scalar(79, 75, 0), Scalar(134, 255, 255), *frame);
        cv::Mat kernel = cv::Mat::ones(5, 5, CV_8UC1);
        detectEdges(frame);
        regionOfInterest(frame);
        std::vector<Vec4i> lines = HoughTransform(frame);
        vector<double> average_x = slope_and_y_intersects(lines, frame);
        cv::imshow("video", test);
        if (cv::waitKey(1) >= 0) break;
    }
    return 0;
}