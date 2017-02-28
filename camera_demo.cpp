
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <gphoto2/gphoto2.h>
#include <libraw.h>
#include <stack>

using namespace std;

/*
 * Load a raw image that was taken with a camera and returns an OpenCV matrix
 * representing the image.  The code for this problem relies on the following
 * code.
 *
 * http://stackoverflow.com/q/22355491/2465202
 */
int load_image(const std::string & path, cv::Mat & output)
{
    LibRaw RawProcessor;

    RawProcessor.imgdata.params.use_camera_wb = 1;
    RawProcessor.imgdata.params.use_auto_wb = 0;

    int ret = RawProcessor.open_file(path.c_str());
    if (ret != LIBRAW_SUCCESS)
    {
        cerr << path.c_str() << " " << libraw_strerror(ret) << endl;
        return(-1);
    }
    ret = RawProcessor.unpack();
    if (ret != LIBRAW_SUCCESS) {
        return(-1);
    }

    int check = RawProcessor.dcraw_process();
    libraw_processed_image_t *image_ptr = RawProcessor.dcraw_make_mem_image(&check);

    output = cv::Mat(cv::Size(image_ptr->width, image_ptr->height), CV_8UC3, image_ptr->data, cv::Mat::AUTO_STEP);
    cv::cvtColor(output, output, 4);
    return(0);
}

/*
 * Takes a gphoto2 camera and context and uses them to capture an image and
 * save them to a file with the given name.  Relies on gp_camera_capture,
 * gp_camera_file_get, gp_file_free, gp_camera_file_delete to trigger the
 * camera, save the image from the camera, and finally delete the image from
 * the camera.  Does not do anything to adjust camera settings currently, or
 * any error checking on the return value from the gphoto calls.  This borrows
 * heavily from:
 * https://github.com/gphoto/libgphoto2/blob/master/examples/sample-capture.c
 */
int capture_image(Camera * camera, GPContext *context, const std::string & name) {
    // Cause the camera to capture
    CameraFile *file;
    CameraFilePath camera_file_path;
    strcpy(camera_file_path.folder, "/");
    strcpy(camera_file_path.name, name.c_str());
    int retval = gp_camera_capture(camera, GP_CAPTURE_IMAGE, &camera_file_path, context);

    // Load the file from the camera
    int fd = open(name.c_str(), O_CREAT | O_WRONLY, 0644);
    retval = gp_file_new_from_fd(&file, fd);
    retval = gp_camera_file_get(camera, camera_file_path.folder, camera_file_path.name,
                                GP_FILE_TYPE_NORMAL, file, context);

    gp_file_free(file);
    retval = gp_camera_file_delete(camera, camera_file_path.folder, camera_file_path.name,
                                   context);
    close(fd);
    return(retval);
}

int main(int argc, char ** argv) {
    // This main function initially took from the demo for the cv::VideoCapture
    // documentation:
    // http://docs.opencv.org/2.4/modules/highgui/doc/reading_and_writing_images_and_video.html#videocapture

    // Setup the camera context, and get the first available camera.  Nothing
    // right now is done to check if the correct camera is found.
    Camera * camera;
    gp_camera_new(&camera);
    GPContext * context = gp_context_new();
    int retval = gp_camera_init(camera, context);
    if (retval != GP_OK) {
        cout << "opening the camera failed: " << retval << endl;
        return(1);
    }

    // The name the raw image takes on disk as it is saved from the camera.
    const string camera_filename = "foo.nef";
    // The name of the OpenCV window.
    const string window_name = "Camera Demo";

    // A quick container to contain so basic hardcoded images.
    stack<string> image_files;
    image_files.push("/Users/david/Desktop/camera_demo/resources/checkerboard.jpg");
    image_files.push("/Users/david/Desktop/camera_demo/resources/colorbars.jpg");

    // Setup the window
    cv::namedWindow(window_name);

    // For each file in the stack do two things
    while (!image_files.empty()) {
        // First read the image that we want to display
        cv::Mat image;
        image = cv::imread(image_files.top());
        cv::Mat image_small;
        cv::resize(image, image_small, cv::Size(1600,1069));
        imshow(window_name, image_small);
        // wait for a key press
        for(;;) {
            if(cv::waitKey(30) >= 0) break;
        }

        // Then capture and image using the camera
        capture_image(camera, context, camera_filename);
        // Process the raw
        load_image(camera_filename, image);
        // And display it
        cv::resize(image, image_small, cv::Size(1600,1069));
        imshow(window_name, image_small);
        // Wait for a key press again before continuing to the next image.
        for(;;) {
            if(cv::waitKey(30) >= 0) break;
        }
        image_files.pop();
    }

    // Clean up and exit.
    gp_camera_exit(camera, context);
    return(0);
}
