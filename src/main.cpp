#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>

using namespace cv;
using namespace std;

void findCircles(Mat& image)
{
    cout << "Finding circles...\n";
    Mat imageGrayscale;

    cvtColor(image, imageGrayscale, COLOR_BGR2GRAY);
    GaussianBlur(imageGrayscale, imageGrayscale, Size(3, 3), 2, 2 );
    vector<Vec3f> circles;
    HoughCircles(
        imageGrayscale,
        circles,
        HOUGH_GRADIENT,
        2.,
        min(imageGrayscale.rows, imageGrayscale.cols)/4.,
        300.,
        200.);
    
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

    imshow("Circles", image);
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

        findCircles(frame);
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
