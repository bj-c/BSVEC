#include "stdafx.h"

CVideo::CVideo()
{
}
CVideo::~CVideo()
{
}
bool CVideo::OpenFileData()
{
	OPENFILENAME ofn;
	//	//select image source
	char input_szFile[MAX_PATH] = "";
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = input_szFile;
	ofn.nMaxFile = sizeof(input_szFile);
	ofn.lpstrFilter = _T("All Files (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (::GetOpenFileName(&ofn) == false)
	{
		return false;
	}
//	printf("%s", input_szFile);
	video_infomation_ = new VideoCapture(input_szFile);

	if (!video_infomation_->isOpened())
	{
		cout << "Error:can't open video file" << endl;
		return false;
	}
	return true;
}
bool CVideo::CloseFileData()
{
	if (video_infomation_ != NULL)
	{
		delete video_infomation_;
		video_infomation_ = NULL;
	}
	return true;
}
CVideoInfo::CVideoInfo()
{
}
CVideoInfo::~CVideoInfo()
{
}
bool CVideoInfo::ReadInputVideo(VideoCapture* video_info, int mode)
{
	index_ = (int)video_info->get(CV_CAP_PROP_FRAME_COUNT);
	height_ = (int)video_info->get(CV_CAP_PROP_FRAME_HEIGHT);
	width_ = (int)video_info->get(CV_CAP_PROP_FRAME_WIDTH);
	fps_ = (int)(800 / video_info->get(CV_CAP_PROP_FPS));

	if (height_ == NULL || width_ == NULL || index_ == NULL || fps_ == NULL)
	{
		cout << "The video info data arise error." << endl;
		return false;
	}
	frame_ = new Mat[index_];

	//Video frame copy
	for (int i = 0; i < index_; i++)
	{
		//Save source frame
		*video_info >> frame_[i];

		if (mode == GRAY)cvtColor(frame_[i], frame_[i], COLOR_BGR2GRAY);

		if (frame_[i].empty())
		{
			cout << "video empty arise error." << endl;
			return false;
		}
	}
	return true;
}
VideoProcessing::VideoProcessing()
{
}

VideoProcessing::~VideoProcessing()
{
}
void VideoProcessing::CorruptImage(Mat targetImage, Mat maskImage)
{
	for (int j = 0; j < targetImage.rows; j++){
		for (int i = 0; i < targetImage.cols; i++){
			if (maskImage.at<uchar>(j, i) == PIXEL_MIN){
				targetImage.at<Vec3b>(j, i)[0] = PIXEL_MIN;
				targetImage.at<Vec3b>(j, i)[1] = PIXEL_MAX;
				targetImage.at<Vec3b>(j, i)[2] = PIXEL_MIN;
			}
		}
	}
}
Mat VideoProcessing::Labeling(Mat target)
{
	Mat mask;
	bitwise_not(target, mask);
	Mat img_labels, stats, centroids;
	int num_of_lables = connectedComponentsWithStats(mask, img_labels, stats, centroids, 4, CV_32S);
	cvtColor(mask, mask, COLOR_GRAY2BGR);

	for (int j = 1; j < num_of_lables; j++){
		int area = stats.at<int>(j, CC_STAT_AREA);
		int left = stats.at<int>(j, CC_STAT_LEFT);
		int top = stats.at<int>(j, CC_STAT_TOP);
		int width = stats.at<int>(j, CC_STAT_WIDTH);
		int height = stats.at<int>(j, CC_STAT_HEIGHT);

		rectangle(mask, Point(left, top), Point(left + width, top + height), Scalar(PIXEL_MIN, PIXEL_MIN, PIXEL_MAX), 1);
		putText(mask, to_string(j), Point(left + 20, top + 20), FONT_HERSHEY_SIMPLEX, 1, Scalar(PIXEL_MAX, PIXEL_MIN, PIXEL_MIN), 2);
	}
//	imshow("Labeling", mask);
	return stats;
}
Mat VideoProcessing::Homography(Mat image_target, Mat image_source, Mat mask_target, Mat mask_source)
{
	Mat img_object, img_scene;
	cvtColor(image_source, img_object, CV_RGB2GRAY);
	cvtColor(image_target, img_scene, CV_RGB2GRAY);

	Mat mask_scene = mask_target.clone();
	Mat	mask_object = mask_source.clone();

	int minHessian = 10;
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> keypoints_object, keypoints_scene;
	Mat descriptors_object, descriptors_scene;
	detector->detectAndCompute(img_object, mask_object, keypoints_object, descriptors_object);
	detector->detectAndCompute(img_scene, mask_scene, keypoints_scene, descriptors_scene);

	if (keypoints_object.size() < 4 || keypoints_scene.size() < 4)return Mat();

	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match(descriptors_object, descriptors_scene, matches);
	std::vector< DMatch > good_matches;

	for (int i = 0; i < descriptors_object.rows; i++)
		good_matches.push_back(matches[i]);

	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	for (size_t i = 0; i < good_matches.size(); i++){
		obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
		scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
	}

	Mat H = findHomography(obj, scene, RANSAC);

	if (H.empty())return Mat();

	//https://darkpgmr.tistory.com/80
	double h[9];
	int index = 0;
	for (int j = 0; j < 3; j++)
		for (int i = 0; i < 3; i++)
			h[index++] = H.at<double>(j, i);

	double D = h[0] * h[4] - h[1] * h[3];
	double sx = sqrt((h[0] * h[0]) + (h[3] * h[3]));
	double sy = sqrt((h[1] * h[1]) + (h[4] * h[4]));
	double P = sqrt((h[6] * h[6]) + (h[7] * h[7]));

	if (D <= 0 || sx < 0.1 || sx>4 || sy < 0.1 || sy>4 || P > 0.002)
		return Mat();

	return H;
}

void VideoProcessing::Reconstruction(Mat image_target, Mat image_source, Mat mask_target, Mat mask_source, Mat mask_label)
{
	for (int j = 0; j < image_target.rows; j++){
		for (int i = 0; i < image_target.cols; i++){
			if ((mask_label.at<uchar>(j, i) == PIXEL_MIN && mask_source.at<uchar>(j, i) == PIXEL_MAX)){
				image_target.at<Vec3b>(j, i)[0] = image_source.at<Vec3b>(j, i)[0];
				image_target.at<Vec3b>(j, i)[1] = image_source.at<Vec3b>(j, i)[1];
				image_target.at<Vec3b>(j, i)[2] = image_source.at<Vec3b>(j, i)[2];
				mask_target.at<uchar>(j, i) = PIXEL_MAX;
			}
		}
	}
}

ROI VideoProcessing::CropROI(Mat label, int numOfLables)
{
	ROI crop;
	crop.left = label.at<int>(numOfLables, CC_STAT_LEFT);
	crop.top = label.at<int>(numOfLables, CC_STAT_TOP);
	crop.width = label.at<int>(numOfLables, CC_STAT_WIDTH);
	crop.height = label.at<int>(numOfLables, CC_STAT_HEIGHT);
	return crop;
}
Mat VideoProcessing::CropImage(Mat image, ROI crop, int block)
{
	int left = MAX(0, crop.left - block);
	int top = MAX(0, crop.top - block);
	int	width = MIN(image.cols - left, crop.width + block + block);
	int	height = MIN(image.rows - top, crop.height + block + block);

	Mat subImage = image(Rect(left, top, width, height));
	return subImage;
}
Mat VideoProcessing::MaskOverlap(Mat mask_target, Mat mask_ref)
{
	Mat mask_result(mask_target.size(), CV_8U, Scalar(PIXEL_MAX));

	for (int j = 0; j < mask_target.rows; j++)
		for (int i = 0; i < mask_target.cols; i++)
			if (mask_target.at<uchar>(j, i) == PIXEL_MIN || mask_ref.at<uchar>(j, i) == PIXEL_MIN)
				mask_result.at<uchar>(j, i) = PIXEL_MIN;

	return mask_result;
}
Mat VideoProcessing::MaskCreate(Mat mask_target, Mat mask_warp1, Mat mask_warp2)
{
	Mat mask_result(mask_target.size(), CV_8U, Scalar(PIXEL_MAX));

	for (int j = 0; j < mask_target.rows; j++)
		for (int i = 0; i < mask_target.cols; i++)
			if (mask_target.at<uchar>(j, i) == PIXEL_MIN || mask_warp1.at<uchar>(j, i) == PIXEL_MIN || mask_warp2.at<uchar>(j, i) == PIXEL_MIN)
				mask_result.at<uchar>(j, i) = PIXEL_MIN;

	return mask_result;
}
double VideoProcessing::SumOfSquareError(Mat image_target, Mat image_source, Mat mask_union)
{
	double ssd = 0;
	int count = 0;
	for (int y = 0; y < image_target.rows; y++){
		for (int x =0; x < image_target.cols; x++){
			if (mask_union.at<uchar>(y, x) == PIXEL_MAX){
				ssd += abs(image_target.at<Vec3b>(y, x)[0] - image_source.at<Vec3b>(y, x)[0]);
				ssd += abs(image_target.at<Vec3b>(y, x)[1] - image_source.at<Vec3b>(y, x)[1]);
				ssd += abs(image_target.at<Vec3b>(y, x)[2] - image_source.at<Vec3b>(y, x)[2]);
				count++;
			}
		}
	}
	ssd /= count;
	return ssd;
}
Mat VideoProcessing::LabelMask(Mat mask_target, Mat label, int block, int numOfLables)
{
	Mat mask_result(mask_target.size(), CV_8U, Scalar(PIXEL_MAX));

	int label_left = label.at<int>(numOfLables, CC_STAT_LEFT);
	int label_top = label.at<int>(numOfLables, CC_STAT_TOP);
	int label_width = label.at<int>(numOfLables, CC_STAT_WIDTH) + label_left;
	int label_height = label.at<int>(numOfLables, CC_STAT_HEIGHT) + label_top;

	for (int j = label_top; j < label_height; j++)
		for (int i = label_left; i < label_width; i++)
			if (mask_target.at<uchar>(j, i) == PIXEL_MIN)
				mask_result.at<uchar>(j, i) = PIXEL_MIN;

	ROI crop;
	crop.left = label.at<int>(numOfLables, CC_STAT_LEFT);
	crop.top = label.at<int>(numOfLables, CC_STAT_TOP);
	crop.width = label.at<int>(numOfLables, CC_STAT_WIDTH);
	crop.height = label.at<int>(numOfLables, CC_STAT_HEIGHT);

	int left = MAX(0, crop.left - block);
	int top = MAX(0, crop.top - block);
	int	width = MIN(mask_target.cols - left, crop.width + block + block);
	int	height = MIN(mask_target.rows - top, crop.height + block + block);
	Mat subImage = mask_result(Rect(left, top, width, height));

	return subImage;
}
void VideoProcessing::MultipleVideoErrorConcealment(Mat image_target, Mat *image_source, Mat mask_target, Mat *mask_source, int boundary, int index)
{
	Mat *image_warp = new Mat[index];
	Mat *mask_warp = new Mat[index];
	Mat *image_warp1_local = new Mat[index];
	Mat *mask_warp1_local = new Mat[index];
	Mat *image_warp2_local = new Mat[index];
	Mat *mask_warp2_local = new Mat[index];
	Mat *metrix_global = new Mat[index];
	double *mse1 = new double[index];
	double *mse2 = new double[index];
	int patch_threshold = 28; //threshold of flowchart

	Mat label = Labeling(mask_target);

	for (int k = 0; k < index; k++){
		metrix_global[k] = Homography(image_target, image_source[k], mask_target, mask_source[k]);
		if (!metrix_global[k].empty()){
			warpPerspective(image_source[k], image_warp[k], metrix_global[k], Size(image_source[k].cols, image_source[k].rows));
			warpPerspective(mask_source[k], mask_warp[k], metrix_global[k], Size(mask_source[k].cols, mask_source[k].rows));
		}
	}
	for (int labelOfNum = 1; labelOfNum < label.rows; labelOfNum++){
		double min_distoration = DBL_MAX;
		for (int k = 0; k < index; k++){
			mse1[k] = DBL_MAX;
			mse2[k] = DBL_MAX;
		}
		ROI sub = CropROI(label, labelOfNum);
		Mat image_target_local = CropImage(image_target, sub, boundary);
		Mat mask_target_local = CropImage(mask_target, sub, boundary);
		Mat mask_label = LabelMask(mask_target, label, boundary, labelOfNum);

		for (int k = 0; k < index; k++){
			if (!metrix_global[k].empty()){
				image_warp1_local[k] = CropImage(image_warp[k], sub, boundary);
				mask_warp1_local[k] = CropImage(mask_warp[k], sub, boundary);
			}
		}
		Mat *metrix_local = new Mat[index];
		for (int k = 0; k < index; k++){
			if (!metrix_global[k].empty()){
				metrix_local[k] = Homography(image_target_local, image_warp1_local[k], mask_target_local, mask_warp1_local[k]);

				if (!metrix_local[k].empty()){
					warpPerspective(image_warp1_local[k], image_warp2_local[k], metrix_local[k], Size(image_warp1_local[k].cols, image_warp1_local[k].rows));
					warpPerspective(mask_warp1_local[k], mask_warp2_local[k], metrix_local[k], Size(mask_warp1_local[k].cols, mask_warp1_local[k].rows));
					threshold(mask_warp1_local[k], mask_warp1_local[k], 254, PIXEL_MAX, CV_THRESH_BINARY);
					threshold(mask_warp2_local[k], mask_warp2_local[k], 254, PIXEL_MAX, CV_THRESH_BINARY);
				}
			}
		}
		Mat *warp_temp = new Mat[index];
		Mat *mask_temp = new Mat[index];

		for (int k = 0; k < index; k++){
			Mat mask_union = mask_target_local.clone();
			
			if (!metrix_global[k].empty()){
				if (!metrix_local[k].empty()){
					mask_union = MaskOverlap(mask_union, mask_warp1_local[k]);
					mask_union = MaskOverlap(mask_union, mask_warp2_local[k]);
					double mse1 = SumOfSquareError(image_target_local, image_warp1_local[k], mask_union);
					double mse2 = SumOfSquareError(image_target_local, image_warp2_local[k], mask_union);

					if (mse1 < mse2){
						warp_temp[k] = image_warp1_local[k].clone();
						mask_temp[k] = mask_warp1_local[k].clone();
					}
					else{
						warp_temp[k] = image_warp2_local[k].clone();
						mask_temp[k] = mask_warp2_local[k].clone();
					}
				}
				else{
					warp_temp[k] = image_warp1_local[k].clone();
					mask_temp[k] = mask_warp1_local[k].clone();
				}
			}
		}
		Mat mask_total = mask_target_local.clone();
		for (int k = 0; k < index; k++){
			if (!mask_temp[k].empty())
				mask_total = MaskOverlap(mask_total, mask_temp[k]);
		}

		int min_patch = index;
		for (int k = 0; k < index; k++){
			if (!mask_temp[k].empty()){
				double mse = SumOfSquareError(image_target_local, warp_temp[k], mask_total);
				if (mse < min_distoration){
					min_distoration = mse;
					min_patch = k;
				}
			}
		}
		if (min_patch != index){
			if (patch_threshold > min_distoration){
				Reconstruction(image_target_local, warp_temp[min_patch], mask_target_local, mask_temp[min_patch], mask_label);
			}
		}
		delete [] warp_temp;
		delete [] mask_temp;
		delete [] metrix_local;
	}
delete [] image_warp;
delete [] mask_warp;
delete [] image_warp1_local;
delete [] mask_warp1_local;
delete [] image_warp2_local;
delete [] mask_warp2_local;
delete [] metrix_global;
delete [] mse1;
delete [] mse2;
}

void VideoProcessing::ImageInpainting(InputArray targetImage, InputArray maskImage)
{
	Mat img_target = targetImage.getMat();
	Mat mask = maskImage.getMat();
	inpaint(img_target, mask, img_target, 10, INPAINT_NS);
	imshow("Image Inpainting", img_target);
	waitKey(30);
}

void VideoProcessing::BilinearInterpolation(InputArray targetImage, InputArray maskImage)
{
	Mat img_target = targetImage.getMat();
	Mat mask = maskImage.getMat();
	int height = img_target.rows;
	int width = img_target.cols;

	double weight[PIXEL_POSITION_COUNT] = { 0,0,0,0 };
	int offset_count[PIXEL_POSITION_COUNT] = { 0, 0, 0, 0 };
	bool ch[PIXEL_POSITION_COUNT] = { 1, 1, 1, 1 };

	for (int j = 0; j < height; j++){
		for (int i = 0; i < width; i++){
			// Find Left
			int pixel[PIXEL_POSITION_COUNT] = { i,i, j, j };

			if (mask.at<uchar>(j, i) == PIXEL_MAX){
				while (pixel[PIXEL_LEFT] >= PIXEL_MIN){
					if (pixel[PIXEL_LEFT] == PIXEL_MIN){
						ch[PIXEL_LEFT] = false;
						break;
					}
					else if (mask.at<uchar>(j, pixel[PIXEL_LEFT]) == PIXEL_MAX){
						pixel[PIXEL_LEFT]--;
						offset_count[PIXEL_LEFT]++;
					}
					else
						break;
				}
				// Find Right
				while (pixel[PIXEL_RIGHT] < width){
					if (pixel[PIXEL_RIGHT] == width - 1){
						ch[PIXEL_RIGHT] = false;
						break;
					}
					else if (mask.at<uchar>(j, pixel[PIXEL_RIGHT]) == PIXEL_MAX){
						pixel[PIXEL_RIGHT]++;
						offset_count[PIXEL_RIGHT]++;
					}
					else
						break;
				}
				// Find Top
				while (pixel[PIXEL_TOP] >= PIXEL_MIN){
					if (pixel[PIXEL_TOP] == PIXEL_MIN){
						ch[PIXEL_TOP] = false;
						break;
					}
					else if (mask.at<uchar>(pixel[PIXEL_TOP], i) == PIXEL_MAX){
						pixel[PIXEL_TOP]--;
						offset_count[PIXEL_TOP]++;
					}
					else
						break;
				}
				// Find Bottom
				while (pixel[PIXEL_BOTTOM] < height){
					if (pixel[PIXEL_BOTTOM] == height - 1){
						ch[PIXEL_BOTTOM] = false;
						break;
					}
					else if (mask.at<uchar>(pixel[PIXEL_BOTTOM], i) == PIXEL_MAX){
						pixel[PIXEL_BOTTOM]++;
						offset_count[PIXEL_BOTTOM]++;
					}
					else
						break;
				}
				int bwith = offset_count[PIXEL_LEFT] + offset_count[PIXEL_RIGHT];
				int bheight = offset_count[PIXEL_TOP] + offset_count[PIXEL_BOTTOM];

				weight[PIXEL_RIGHT] = bwith - offset_count[PIXEL_RIGHT];
				weight[PIXEL_LEFT] = bwith - offset_count[PIXEL_LEFT];
				weight[PIXEL_BOTTOM] = bheight - offset_count[PIXEL_BOTTOM];
				weight[PIXEL_TOP] = bheight - offset_count[PIXEL_TOP];

				double temp[3] = { 0.0, 0.0, 0.0 };
				double weight_m = 0;

				if (ch[PIXEL_LEFT]){
					temp[0] += img_target.at<Vec3b>(j, pixel[PIXEL_LEFT])[0] * weight[PIXEL_LEFT];
					temp[1] += img_target.at<Vec3b>(j, pixel[PIXEL_LEFT])[1] * weight[PIXEL_LEFT];
					temp[2] += img_target.at<Vec3b>(j, pixel[PIXEL_LEFT])[2] * weight[PIXEL_LEFT];
					weight_m += weight[PIXEL_LEFT];
				}
				if (ch[PIXEL_RIGHT]){
					temp[0] += img_target.at<Vec3b>(j, pixel[PIXEL_RIGHT])[0] * weight[PIXEL_RIGHT];
					temp[1] += img_target.at<Vec3b>(j, pixel[PIXEL_RIGHT])[1] * weight[PIXEL_RIGHT];
					temp[2] += img_target.at<Vec3b>(j, pixel[PIXEL_RIGHT])[2] * weight[PIXEL_RIGHT];
					weight_m += weight[PIXEL_RIGHT];
				}
				if (ch[PIXEL_TOP]){
					temp[0] += img_target.at<Vec3b>(pixel[PIXEL_TOP], i)[0] * weight[PIXEL_TOP];
					temp[1] += img_target.at<Vec3b>(pixel[PIXEL_TOP], i)[1] * weight[PIXEL_TOP];
					temp[2] += img_target.at<Vec3b>(pixel[PIXEL_TOP], i)[2] * weight[PIXEL_TOP];
					weight_m += weight[PIXEL_TOP];
				}
				if (ch[PIXEL_BOTTOM]){
					temp[0] += img_target.at<Vec3b>(pixel[PIXEL_BOTTOM], i)[0] * weight[PIXEL_BOTTOM];
					temp[1] += img_target.at<Vec3b>(pixel[PIXEL_BOTTOM], i)[1] * weight[PIXEL_BOTTOM];
					temp[2] += img_target.at<Vec3b>(pixel[PIXEL_BOTTOM], i)[2] * weight[PIXEL_BOTTOM];
					weight_m += weight[PIXEL_BOTTOM];
				}
				for (int s = 0; s < 3; s++){
					temp[s] /= weight_m;
					if (temp[s] > PIXEL_MAX)
						temp[s] = PIXEL_MAX;
					else if (temp[s] < 0)
						temp[s] = 0;
				}
				img_target.at<Vec3b>(j, i)[0] = (uchar)temp[0];
				img_target.at<Vec3b>(j, i)[1] = (uchar)temp[1];
				img_target.at<Vec3b>(j, i)[2] = (uchar)temp[2];
			}
		}
	}
	imshow("BilinearInterpolation", img_target);
	waitKey(30);
}
void VideoProcessing::TemporalReplacement(InputArray targetImage, InputArray referenceImage, InputArray maskImage)
{
	Mat img_target = targetImage.getMat();
	Mat img_reference = referenceImage.getMat();
	Mat mask = maskImage.getMat();

	for (int j = 0; j < img_target.rows; j++){
		for (int i = 0; i < img_target.cols; i++){
			if (mask.at<uchar>(j, i) == PIXEL_MAX){
				img_target.at<Vec3b>(j, i)[0] = img_reference.at<Vec3b>(j, i)[0];
				img_target.at<Vec3b>(j, i)[1] = img_reference.at<Vec3b>(j, i)[1];
				img_target.at<Vec3b>(j, i)[2] = img_reference.at<Vec3b>(j, i)[2];
			}
		}
	}
	imshow("Temporal Replacement", img_target);
	waitKey(30);
}
void VideoProcessing::SaveImageAndVideo(Mat* targetFrame, int width, int height, int count, double fps)
{
	//Image Save
	char *outputfilename = new char[200];
	for (int k = 0; k < count; k++)
	{
		sprintf_s(outputfilename, sizeof(outputfilename), "%d.bmp", k);
		imwrite(outputfilename, targetFrame[k]);
	}
	delete[] outputfilename;

	//video save
	int fourcc = VideoWriter::fourcc('F', 'F', 'V', '1'); // opencv3
	bool isColor = true;
	VideoWriter *videoSave = new VideoWriter;
	if (!videoSave->open("ResultVideo.avi", fourcc, fps, Size(width, height), isColor))
	{
		delete videoSave;
		return;
	}
	for (int k = 0; k < count; k++)
	{
		*videoSave << targetFrame[k];
	}
	delete videoSave;
}
double VideoProcessing::getPSNR(const Mat& I1, const Mat& I2, int count)
{
	Mat s1;
	absdiff(I1, I2, s1);       // |I1 - I2|
	s1.convertTo(s1, CV_32F);  // cannot make a square on 8 bits

	s1 = s1.mul(s1);           // |I1 - I2|^2

	Scalar s = sum(s1);         // sum elements per channel

	double sse = s.val[0] + s.val[1] + s.val[2]; // sum channels

	if (sse <= 1e-10) // for small values return zero
		return 0;
	else
	{
		double  mse = sse / (double)(I1.channels() * count);
		double psnr = 10.0*log10((PIXEL_MAX * PIXEL_MAX) / mse);
		return psnr;
	}
}
double VideoProcessing::getSSIM(const Mat& i1, const Mat& i2)
{
	const double C1 = 6.5025, C2 = 58.5225;
	/***************************** INITS **********************************/
	int d = CV_32F;

	Mat I1, I2;
	i1.convertTo(I1, d);            // cannot calculate on one byte large values
	i2.convertTo(I2, d);

	Mat I2_2 = I2.mul(I2);        // I2^2
	Mat I1_2 = I1.mul(I1);        // I1^2
	Mat I1_I2 = I1.mul(I2);        // I1 * I2

   /*************************** END INITS **********************************/

	Mat mu1, mu2;                   // PRELIMINARY COMPUTING
	GaussianBlur(I1, mu1, Size(11, 11), 1.5);
	GaussianBlur(I2, mu2, Size(11, 11), 1.5);

	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);

	Mat sigma1_2, sigma2_2, sigma12;

	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;

	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;

	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;

	///////////////////////////////// FORMULA ////////////////////////////////
	Mat t1, t2, t3;

	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);                 // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))

	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);                 // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))

	Mat ssim_map;
	divide(t3, t1, ssim_map);        // ssim_map =  t3./t1;

	Scalar mssim = mean(ssim_map);   // mssim = average of ssim map

	double temp = mssim.val[0] + mssim.val[1] + mssim.val[2];

	double averageSSIM = temp / (double)(i1.channels());
	return averageSSIM;
}
void VideoProcessing::EvaluationPSNR(Mat* targetFrame, Mat* originalFrame, Mat* mask, int vcount)
{
	double HallPSNR = 0;

	for (int k = 0; k < vcount; k++)
	{
		int count = 0;

		for (int j = 0; j < mask[k].rows; j++)
			for (int i = 0; i < mask[k].cols; i++)
				if (mask[k].at<uchar>(j, i) == PIXEL_MAX)count++;

		double psnrS = getPSNR(targetFrame[k], originalFrame[k], count);
		HallPSNR += psnrS;
	}
	HallPSNR = HallPSNR / vcount;
	cout << "PSNR:" << setprecision(4) << HallPSNR << "dB" << endl;
}

void VideoProcessing::EvaluationSSIM(Mat* targetFrame, Mat* originalFrame, Mat* mask, int vcount)
{
	double FullSSIM = 0;
	for (int k = 0; k < vcount; k++){
		double ssimV = getSSIM(targetFrame[k], originalFrame[k]);
		FullSSIM += ssimV;
	}
	FullSSIM = FullSSIM / vcount;
	cout << "SSIM:" << setprecision(4) << FullSSIM << "dB" << endl;
}
