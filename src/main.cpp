#include <iostream>
#include <future>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>

using namespace cv;
using namespace std;

static Mat ColorToGray(const Mat& image)
{
    Mat imageGS;
    cvtColor(image, imageGS, COLOR_BGR2GRAY);
    return imageGS;
}

// basic ORB keypoints detection - returns detected keypoints
vector<KeyPoint> findCorners(const Mat& image)
{
    cout << "Finding corners...\n";

    const auto imageGrayscale = ColorToGray(image);

    Mat corners;
    vector<KeyPoint> keypoints_object;

    auto detector = ORB::create();
    detector->detect(imageGrayscale, keypoints_object);

    return keypoints_object;
}

// optical flow calculation
// tweaked snippet from 'https://docs.opencv.org/3.4/d4/dee/tutorial_optical_flow.html'
vector<Point2f> calcOpticalFlow(
    const Mat& framePrevious,
    const Mat& frameCurrent,
    Mat& mask,
    const vector<Point2f>& pointsPrevious)
{
    vector<Point2f> pointsCurrent;

    auto framePreviousGS = ColorToGray(framePrevious);
    auto frameCurrentGS = ColorToGray(frameCurrent);
    auto frameCopy = frameCurrent.clone();

    // Calculate optical flow
    vector<uchar> status;
    vector<float> err;
    TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), 10, 0.03);
    calcOpticalFlowPyrLK(
        framePreviousGS,
        frameCurrentGS,
        pointsPrevious,
        pointsCurrent,
        status,
        err,
        Size(15,15),
        2,
        criteria);

    // Create random colors
    //TODO initialize only once outside of this function
    vector<Scalar> colors;
    RNG rng;
    const auto maxPoints = 100;
    for(int i = 0; i < maxPoints; i++)
    {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r,g,b));
    }

    // store new best matches here - these will be returned to the main process
    vector<Point2f> newGoodPoints;

    // Visualization part
    //TODO pack this in its own function
    for(uint i = 0; i < pointsPrevious.size() || i < maxPoints; i++)
    {
        // Select good points
        if(status[i] == 1)
        {
            newGoodPoints.push_back(pointsCurrent[i]);
            // Draw the tracks
            line(mask, pointsCurrent[i], pointsPrevious[i], colors[i], 2);
            circle(frameCopy, pointsCurrent[i], 5, colors[i], -1);
        }
    }

    Mat frameFlow;
    add(frameCopy, mask, frameFlow);
    imshow("Optical flow", frameFlow);

    cout << newGoodPoints.size() << " good matches found\n";

    return newGoodPoints;
}

// draws provided circles and centers on the provided image
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

// detect circles on an image and returns the detected circles parameters
vector<Vec3f> findCircles(const Mat& image)
{
    cout << "Finding circles...\n";

    const auto imageGrayscale = ColorToGray(image);

    Mat imageBlurred;
    GaussianBlur(imageGrayscale, imageBlurred, Size(3, 3), 2, 2);

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

Mat ResetTrackingState(const Mat& frame, vector<Point2f>& points)
{
    auto frameGS = ColorToGray(frame);
    goodFeaturesToTrack(frameGS, points, 100, 0.3, 7, Mat(), 7, false, 0.04);

    // Create a mask image for drawing purposes
    return Mat::zeros(frame.size(), frame.type());
}

// main camera capturing loop
bool processCam()
{
    Mat frameCurrent;
    Mat framePrevious;
    vector<Point2f> pointsPrevious;

    VideoCapture cap;
    cap.open(0);

    cout << "Testing camera...\n";

    if (!cap.isOpened())
    {
        cout << "Camera not available\n";
        return false;
    }

    cout << "Capturing...\n";

    // initialize previous frame and features to bootstrap tracking
    cap.read(framePrevious);

    if (framePrevious.empty())
    {
        cerr << "ERROR! blank frame grabbed\n";
        return false;
    }

    auto mask = ResetTrackingState(framePrevious, pointsPrevious);

    auto processedFrames = 0;
    const auto maxProcessedFrames = 5;

    while (true)
    {
        cap.read(frameCurrent);

        if (frameCurrent.empty())
        {
            //TODO consider retrying a few times before giving up
            cerr << "ERROR! blank frame grabbed\n";
            break;
        }

        // spawn parallel processes to handle various computations
        auto f1 = async(findCircles,ref(frameCurrent));
        auto f2 = async(findCorners,ref(frameCurrent));
        auto f3 = async(
            calcOpticalFlow,
            ref(framePrevious),
            ref(frameCurrent),
            ref(mask),
            pointsPrevious);

        // retrieve and use results from the other processes when available
        auto frameCopy = frameCurrent.clone();
        drawCircles(frameCopy, f1.get());
        drawKeypoints(frameCopy, f2.get(), frameCopy, cv::Scalar(0,255,255));

        //TODO pack this in its own function/class
        {
            pointsPrevious = f3.get();
            framePrevious = frameCurrent.clone();

            // prevent tracking for too long
            if (++processedFrames == maxProcessedFrames)
            {
                cout << "Resetting tracking state...\n";
                processedFrames = 0;
                mask = ResetTrackingState(framePrevious, pointsPrevious);
            }
        }

        //TODO merge this with optical flow view
        imshow("Live view", frameCopy);

        if (waitKey(5) >= 0)
            break;
    }

    cout << "Done\n";
    return true;
}

// fall back function for testing algorithms on a static image
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
