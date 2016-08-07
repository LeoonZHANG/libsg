/*
 * img_cv.cpp
 * Author: wangwei.
 * Test for image util module img_cv.
 */

void test_clone(const char *src_img_path,
                const char *save_img_path)
{
    img_cv *src, *src_clone;
    uint8_t *src_data;
    uint64_t src_data_size;

    src = img_cv_load_file(src_img_path);
    src_clone = img_cv_clone(src);

    src_data = img_cv_data(src);
    src_data_size = img_cv_data_size(src);
    memset(src_data, 0, src_data_size);
    img_cv_free(&src);

    img_cv_save(src_clone, save_img_path);
    img_cv_free(&src_clone);
}

void test_2d_3d_rotate(const char *src_img_path,
                       const char *save_dir_2d,
                       const char *save_dir_3d)
{
    int i;
    img_cv *src, *dst_2d, *dst_3d;
    char save_path_2d[1024], save_path_3d[1024];

    src = img_cv_load_file(src_img_path);

    for(i = -90; i <= 90; i++) {
        sprintf(save_path_2d, "%s/%d.png", save_dir_2d, i + 90);
        sprintf(save_path_3d, "%s/%d.png", save_dir_3d, i + 90);
        dst_2d = img_cv_2d_rotate(src, (float)i);
        dst_3d = img_cv_3d_rotate(src, (float)i);
        if(!dst_2d || !dst_3d) {
            assert(0);
            continue;
        }
        img_cv_save(dst_2d, save_path_2d);
        img_cv_save(dst_3d, save_path_3d);
        img_cv_free(&dst_2d);
        img_cv_free(&dst_3d);
    }

    img_cv_free(&src);
}

void test_resize_trim(void)
{
    img_cv *src, *src_under, *dst, *dst2, *dst_blended;

    src = img_cv_load_file("/home/metalwood/zaojiao.png");
    if(src == NULL)
        return;

    dst = img_cv_resize(src, 242, 163);
    if(dst == NULL)
        return;

    dst2 = img_cv_trim(dst, 10, 10, 10, 10);

    img_cv_save(dst, "/home/metalwood/zaojiao_resized.png");
    img_cv_save(dst2, "/home/metalwood/zaojiao_trimed.png");

    src_under = img_cv_load_file("/home/metalwood/zaojiao.png");
    dst_blended = img_cv_blend(src, src_under, 10, 10);
    img_cv_save(dst_blended, "/home/metalwood/zaojiao_blend.png");
}

void test_blend_a(void)
{
    cv::Mat underlay = cv::imread("/home/metalwood/zaojiao.png", CV_LOAD_IMAGE_UNCHANGED);
    cv::Mat overlay = cv::imread("/home/metalwood/rect.fw.png", CV_LOAD_IMAGE_UNCHANGED);
    cv::Mat roi;

    roi = underlay(cv::Rect(10, 100, overlay.cols, overlay.rows));
    //cv::addWeighted(roi, 0, overlay, 1, 0, roi); //replace , == overlay.copyTo(roi);
    //cv::add(roi, overlay, roi);
    cv::merge(&overlay, 4, roi);
    cv::imwrite("/home/metalwood/zaojiao_blend_a.png", underlay);
}

void test_blend_b(void)
{
    cv::Mat underlay = cv::imread("/home/metalwood/zaojiao.png", CV_LOAD_IMAGE_UNCHANGED);
    cv::Mat overlay = cv::imread("/home/metalwood/test pngs/s.8bit-2clr-alptrans.png", CV_LOAD_IMAGE_UNCHANGED);
    cv::Mat roi; /* Region of interest. */

    roi = underlay(cv::Rect(10, 10, overlay.cols, overlay.rows));
    overlay.copyTo(roi);
    cv::imwrite("/home/metalwood/zaojiao_blend.png", underlay);
}

img_cv *test_blend_c(img_cv *overlay, img_cv *underlay, int x, int y)
{
    cv::Mat *overlay_mat = (cv::Mat *)overlay;
    cv::Mat *underlay_mat = (cv::Mat *)underlay;
    cv::Mat *res;
    double alpha = 0.5; /* Do not change alpha degree. */
    double beta = 1.0 - alpha;

    assert(overlay_mat);
    assert(underlay_mat);
    assert(overlay_mat->data);
    assert(underlay_mat->data);

    res = new cv::Mat();
    if(res == NULL) {
        fprintf(stderr, "New cv::Mat failure.");
        return NULL;
    }

    cv::addWeighted(*overlay_mat, alpha, *underlay_mat, 1 - alpha, 0, *res);

    return res;
}

void test_set_alpha(const char *src_path, float alpha, const char *dst_path)
{
    img_cv *src_img, *dst_img;

    src_img = img_cv_load_file(src_path);
    dst_img = img_cv_set_alpha(src_img, alpha);
    img_cv_save(dst_img, dst_path);
}

void test_transparent(void)
{
    /* Big Red rectangle. */
    cv::Mat image(400, 400, CV_8UC4, cv::Scalar(0, 0, 200)/* Init color 0, 0, 200. */);
    cv::Mat channels[4]; /* B->G->R->A channels. */

    /* split channels to different Mat. */
    cv::split(image, channels);//imageBlue = channels.at(0);

    /* Draw a rectangle on Alpha channel. */
    cv::rectangle(channels[3], cv::Rect(100, 100, 200, 200), cv::Scalar(255), -1);

    cv::merge(channels, 4, image);
    cv::imwrite("transparent1.png", image);

    cv::rectangle(channels[3], cv::Rect(150, 150, 100, 100), cv::Scalar(127), -1);
    cv::merge(channels, 4, image);
    cv::imwrite("transparent2.png", image);

    channels[3] = channels[3] * 0.5; // here you can change transparency %50
    // channels[3] = channels[3] + 50 // you can add some value
    // channels[3] = channels[3] - 50 // you can subtract some value

    cv::merge(channels, 4, image);
    cv::imwrite("transparent3.png", image);

    cv::waitKey(0);
}

int main(int argc, char *argv[])
{
    test_set_alpha("/home/metalwood/test pngs/s.8bit-2clr-alptrans.png",
                   0.6,
                   "/home/metalwood/s.24bit_set_alpha.png");

    test_blend_a();

    test_transparent();

    test_clone("/home/metalwood/ewm.png", "/home/metalwood/ewm_clone.png");

    test_2d_3d_rotate("/home/metalwood/ewm.png",
                      "/home/metalwood/2d_rotate",
                      "/home/metalwood/3d_rotate");

    test_resize_trim();

    return 0;
}