#include "opencv2/core.hpp"
#include "opencv2/core/hal/interface.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/highgui.hpp"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Add Gaussian Noise
cv::Mat addGaussianNoise(cv::Mat image, double mean, double stddev) {
    cv::Mat noise(image.size(), image.type());
    cv::randn(noise, mean, stddev);
    cv::Mat result = image + noise;
    return result;
}

struct Options {
    std::string imagePath;
    int sampleSize = 5;
    bool showRandom = false;
};

Options parseArgs(int argc, char *argv[]) {
    Options opts;
    for (int i = 0; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-i" || arg == "--image") && i + 1 < argc) {
            opts.imagePath = argv[++i];
        } else if (arg == "--sample-size" && i + 1 < argc) {
            opts.sampleSize = std::stoi(argv[++i]);
        } else if (arg == "--show-random-noisy-image") {
            opts.showRandom = true;
        }
    }

    return opts;
}

int main(int argc, char *argv[]) {
    auto opts = parseArgs(argc, argv);

    if (opts.imagePath.empty()) {
        std::cerr << "Usage: " << argv[0]
                  << " -i <image_path> [--samples-size N] [--show-random-noisy-image]\n";

        return 1;
    }

    cv::Mat img = cv::imread(opts.imagePath, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error: No image found.\n";
        return -1;
    }

    // Convert image to grayscale
    cv::Mat grayImage;
    cv::cvtColor(img, grayImage, cv::COLOR_BGR2GRAY);

    // Resize (400x300)
    cv::Mat resized_img;
    cv::Size new_size(800, 600);
    resize(grayImage, resized_img, new_size, 0, 0, cv::INTER_LINEAR);

    // generate noisy samples
    std::vector<cv::Mat> noisy_images;
    for (int i = 0; i < opts.sampleSize; ++i)
        noisy_images.push_back(addGaussianNoise(resized_img, 0, 20));

    /*
     * Noise Reduction
     *
     * Uses Central Limit Theorem to reduce noise in the image
     */
    cv::Mat result(resized_img.size(), CV_32F, cv::Scalar(0));
    for (int i = 0; i < opts.sampleSize; ++i) {
        result += noisy_images[i];
    }
    result /= opts.sampleSize;
    result.convertTo(result, CV_8U);

    // --show-ramdom-noisy-image: true
    // show random noisy sample
    if (opts.showRandom) {
        std::srand(std::time(0));
        int idx = std::rand() % noisy_images.size();
        cv::Mat random_sample;
        noisy_images[idx].convertTo(random_sample, CV_8U);
        cv::imshow("Random Noisy Sample", random_sample);
    }

    cv::imshow("Denoised Result", result);

    int key;
    for (;;) {
        key = cv::waitKey(20);
        if (key == 27 || key == 'q') {
            break;
        }
    }

    return 0;
}
