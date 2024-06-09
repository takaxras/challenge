#include <iostream>
#include <future>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

vector<KeyPoint> findCorners(Mat& image)
{
    cout << "Finding corners...\n";

    Mat imageGrayscale;
    cvtColor(image, imageGrayscale, COLOR_BGR2GRAY);

    Mat corners;
    vector<KeyPoint> keypoints_object;

    auto detector = ORB::create();
    detector->detect(imageGrayscale, keypoints_object);

    return keypoints_object;
}

void drawCircles(Mat& image, const vector<Vec3f>& circles)
{
    const auto nCircles = circles.size();
    cout << nCircles << " detected\n";

    for (const auto& c : circles)
    {
        Point center(cvRound(c[0]), cvRound(c[1]));
        const auto radius = cvRound(c[2]);

        // draw the circle center
        circle(image, center, 3, Scalar(0,255,0), -1, 8, 0);
        // draw the circle outline
        circle(image, center, radius, Scalar(0,0,255), 3, 8, 0);
    }
}

vector<Vec3f> findCircles(Mat& image)
{
    cout << "Finding circles...\n";
    Mat imageGrayscale;
    cvtColor(image, imageGrayscale, COLOR_BGR2GRAY);
    Mat imageBlurred;
    GaussianBlur(imageGrayscale, imageBlurred, Size(3, 3), 2, 2 );
    vector<Vec3f> circles;
    HoughCircles(
        imageBlurred,
        circles,
        HOUGH_GRADIENT,
        2.,
        min(image.rows, image.cols)/4.,
        150.,
        200.);

    return circles;
}

bool processCam()
{
    Mat frame;
    VideoCapture cap;
    cap.open(0);

    cout << "Testing camera...\n";

    if (!cap.isOpened())
    {
        cout << "Camera not available\n";
        return false;
    }

    cout << "Capturing...\n";

    while (true)
    {
        cap.read(frame);

        if (frame.empty())
        {
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        auto f1 = async(findCircles,ref(frame));
        auto f2 = async(findCorners,ref(frame));

        drawCircles(frame, f1.get());
        drawKeypoints(frame, f2.get(), frame, cv::Scalar(0,255,255));
        imshow("Boom", frame);

        if (waitKey(5) >= 0)
            break;
    }

    cout << "Done\n";
    return true;
}

void processImage()
{
    cout << "Loading image...\n";

    auto imgLoaded = imread("./data/planets.png");

    if (imgLoaded.empty())
    {
        cerr << "ERROR! Empty image\n";
        return;
    }

    cout << "Finding circles...\n";
    findCircles(imgLoaded);
    cout << "Done\n";
    waitKey(0);
}

int main()
{
    if (!processCam())
        processImage();

    return 0;
}
