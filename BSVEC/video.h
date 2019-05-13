#pragma once
#include "stdafx.h"

#define PIXEL_MAX 255
#define PIXEL_MIN 0
#define PIXEL_FIX 254
#define COLOR 1
#define GRAY 0

struct ROI
{
	int left;
	int top;
	int width;
	int height;
};
enum PIXEL_POSITION
{
	PIXEL_LEFT,
	PIXEL_RIGHT,
	PIXEL_TOP,
	PIXEL_BOTTOM,
	PIXEL_POSITION_COUNT,
};
class CVideo
{
public:
	CVideo();
	~CVideo();
	bool OpenFileData();
	bool CloseFileData();

	VideoCapture *video_infomation_;
protected:

};

class CVideoInfo
{
public:
	CVideoInfo();
	~CVideoInfo();

	int index_;
	int width_;
	int height_;
	int fps_;
	int type_;
	

	Mat *frame_;

	bool ReadInputVideo(VideoCapture* video_info, int mode);
};


class VideoProcessing
{
public:
	VideoProcessing();
	~VideoProcessing();

	void CorruptImage(Mat targetImage, Mat maskImage);

	void EvaluationPSNR(Mat* target_frame, Mat* original_frame, Mat* mask, int vcount);
	void EvaluationSSIM(Mat* target_frame, Mat* original_frame, Mat* mask, int vcount);
	void SaveImageAndVideo(Mat* target_frame, int width, int height, int count, double fps);

	Mat Labeling(Mat target);

	Mat Homography(Mat image_target, Mat image_source, Mat mask_target, Mat mask_source);
	ROI CropROI(Mat label, int numOfLables);
	Mat CropImage(Mat image, ROI crop, int block);
	Mat MaskCreate(Mat mask_target, Mat mask_warp1, Mat mask_warp2);
	Mat MaskOverlap(Mat mask_target, Mat mask_ref);

	Mat LabelMask(Mat mask_target, Mat label, int block, int numOfLables);
	void Reconstruction(Mat image_target, Mat image_source, Mat mask_target, Mat mask_source, Mat mask_label);
	void MultipleVideoErrorConcealment(Mat image_target, Mat *image_source, Mat mask_target, Mat *mask_source, int boundary, int refCount);
	void BilinearInterpolation(InputArray targetImage, InputArray maskImage);
	void ImageInpainting(InputArray targetImage, InputArray maskImage);
	void TemporalReplacement(InputArray targetImage, InputArray referenceImage, InputArray maskImage);


	double getPSNR(const Mat& I1, const Mat& I2, int count);
	double getSSIM(const Mat& i1, const Mat& i2);
	double SumOfSquareError(Mat image_target, Mat image_source, Mat mask_union);
protected:
};