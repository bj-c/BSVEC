/****************************************************************************
- Software name: Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration
- Software writer: Byungjin Chung and Chunghoon Yim
- OpenCV version: 3.4.1
  This software is the implementation for the following publication
  If you are this software, please refer the following journal publication.
- Institute: Image and Video Processing Laboratory, Konkuk University

  Author:  Byungjin Chung and Changhoon Yim,
  Title:   Bi-sequential Video Error Concealment Method Using Adaptive Homography-based Registration,
  Journal: IEEE Transactions on Circuits and Systems for Video Technology,
  Year =   2019

- Byungjin Chung and Changhoon Yim have developed this software and related documentation (the "Software");
  academic use in source form of the software, without modification, is permitted provided that the following
  conditions are met:
- The use of the software is for academic purpose and is for Non-Commercial Purpose only. As used in this agreement,
  "Non-Commercial Purpose" means for the purpose of research or education in a non-commercial organisation only.
  "Non-Commercial Purpose" excludes, without limitation, any use of the Software for, as part of, or in any way in
  connection with a product (including software) or service which is sold, offered for sale.
- If you have any problem or issue for this software, please email to Byungjin Chung [email:vival@konkuk.ac.kr]
*****************************************************************************/
#include "stdafx.h"

bool SourceAndMask(VideoCapture* src, VideoCapture* mask)
{
	bool src_and_mask = true;

	if (src->get(CV_CAP_PROP_FRAME_COUNT) != mask->get(CV_CAP_PROP_FRAME_COUNT))
	{
		cout << "Error:The video (source and mask) is different (frame index) " << endl;
		src_and_mask = false;
	}
	if (src->get(CV_CAP_PROP_FRAME_WIDTH) != mask->get(CV_CAP_PROP_FRAME_WIDTH))
	{
		cout << "Error:The video (source and mask) is different (frame width) " << endl;
		src_and_mask = false;
	}
	if (src->get(CV_CAP_PROP_FRAME_HEIGHT) != mask->get(CV_CAP_PROP_FRAME_HEIGHT))
	{
		cout << "Error:The video (source and mask) is different (frame height) " << endl;
		src_and_mask = false;
	}
	return src_and_mask;
}

int _tmain(int argc, char* argv[])
{
	CVideo input_source, input_mask;
	CVideoInfo src, mask;
	VideoProcessing process;

	// ========================================================================
	// Load source video, mask video
	// ========================================================================

	cout << "Open input source video file" << endl;
	if (!input_source.OpenFileData())return 1;
	cout << "DONE!!\n" << endl;

	cout << "Open input mask video file" << endl;
	if (!input_mask.OpenFileData())return 1;
	cout << "DONE!!\n" << endl;

	if (!SourceAndMask(input_source.video_infomation_, input_mask.video_infomation_))return 1;

	if (!src.ReadInputVideo(input_source.video_infomation_, COLOR) || !mask.ReadInputVideo(input_mask.video_infomation_, GRAY))return 1;


	//select video
	int method_number;
	cout << "/////////////////////////////////////////////////////////////////////\n"
		<< "// Mail: vival@konkuk.ac.kr, cyim@konkuk.ac.kr                     //\n"
		<< "// Contact: Byungjin Chung and Changhoon Yim                       //\n"
		<< "/////////////////////////////////////////////////////////////////////\n"
		<< "// CopyRight:                                                      //\n"
		<< "// Konkuk University, Image and Video Processing Laboratory        //\n"
		<< "/////////////////////////////////////////////////////////////////////\n"
		<< "\n"
		<< "1. Bi-sequential video error concealment (BSVEC)\n"
		<< "2. Bilinear interpolation (BI)\n"
		<< "3. Diffusion inpainting (DI)\n"
		<< "4. Temporal replacement (TR)\n"
		<< endl
		<< "select method [1-4]: ";
	cin >> method_number;
	// ========================================================================
	// Task of sample video for PSNR and SSIM
	// ========================================================================
	Mat* src_clone = new Mat[src.index_];
	Mat* mask_clone = new Mat[src.index_];

	for (int k = 0; k < src.index_; k++) {
		threshold(mask.frame_[k], mask.frame_[k], 250, PIXEL_MAX, CV_THRESH_BINARY);
		src_clone[k] = src.frame_[k].clone();
		mask_clone[k] = mask.frame_[k].clone();
	}

	// ========================================================================
	// Make damaged source video using mask video
	// ========================================================================

	cout << "Make damaged source video using mask video" << endl;
	for (int k = 0; k < src.index_; k++) {
		bitwise_not(mask.frame_[k], mask.frame_[k]);
		process.CorruptImage(src.frame_[k], mask.frame_[k]);
//		imshow("Damaged Video", src.frame_[k]);
//		waitKey(1);
	}
	cout << "DONE!!\n" << endl;

	clock_t start = clock();
	switch (method_number) {
	case 1:
	{
		cout
			<< "Parameters reading\n"
			<< "Method: Bi-sequential video error concealment (BSVEC)\n"
			<< "Size of width height in video:[" << src.width_ << "," << src.height_ << "]\n"
			<< "Number of frames in video:[" << src.index_ << "]\n"
			<< "Frame per second of video:[" << src.fps_ << "]\n"
			<< endl;

		int boundary;
		int group_of_frame;
		int ref_of_frame;
		int idx = 0;
		int fix = 0;
		cout
			<< "Parameters setting\n"
			<< "Boundary of local patch (ex:64):";
		cin >> boundary;
		cout
			<< "Number of frames in GOF or number of all frames (ex:20 or "<< src.index_<<"):";
		cin >> group_of_frame;
		cout
			<< "Number of reference frames (ex:1):";
		cin >> ref_of_frame;

		int group = group_of_frame;
		int space = ref_of_frame;
		Mat *ref_frame = new Mat[ref_of_frame];
		Mat *ref_mask = new Mat[ref_of_frame];

		while (idx < src.index_)
		{
			while (idx < group - 1)
			{
				idx++;
				if (idx < space)
					ref_of_frame = idx;
				else
					ref_of_frame = space;

				for (int i = 0; i < ref_of_frame; i++) {
					ref_frame[i] = src.frame_[idx - (i + 1)];
					ref_mask[i] = mask.frame_[idx - (i + 1)];
				}
				process.MultipleVideoErrorConcealment(src.frame_[idx], ref_frame, mask.frame_[idx], ref_mask, boundary, ref_of_frame);
				imshow("Bi-sequential Video Error Concealment", src.frame_[idx]);
				waitKey(1);
			}
			// ========================================================================
			// HEISI method url: https://www.sciencedirect.com/science/article/pii/S0923596514001398
			// ========================================================================
			imwrite("last.bmp", src.frame_[idx]);
			system("HEISI.exe LAST.bmp HEISI.bmp");
			src.frame_[idx] = imread("HEISI.bmp");
			mask.frame_[idx] = Scalar(PIXEL_MAX);

			while (idx > fix) {
				idx--;
				if ((group - 1) - idx < space)
					ref_of_frame = (group - 1) - idx;
				else
					ref_of_frame = space;

				for (int i = 0; i < ref_of_frame; i++)
				{
					ref_frame[i] = src.frame_[idx + (i + 1)];
					ref_mask[i] = mask.frame_[idx + (i + 1)];
				}

				process.MultipleVideoErrorConcealment(src.frame_[idx], ref_frame, mask.frame_[idx], ref_mask, boundary, ref_of_frame);

			// ========================================================================
			// HEISI method url: https://www.sciencedirect.com/science/article/pii/S0923596514001398
			// ========================================================================

				imwrite("last.bmp", src.frame_[idx]);
				system("HEISI.exe LAST.bmp HEISI.bmp");
				src.frame_[idx] = imread("HEISI.bmp");
				mask.frame_[idx] = Scalar(PIXEL_MAX);
				imshow("Bi-sequential Video Error Concealment", src.frame_[idx]);
				waitKey(1);
			}
			fix = group;
			idx = group;
			group += group_of_frame;

			if (group >= src.index_) {
				group = src.index_;
			}
		}
		delete[] ref_frame;
		delete[] ref_mask;
	}
	break;
	case 2:
		cout
			<< "Parameters reading\n"
			<< "Method: Bilinear interpolation (BI)\n"
			<< "Video size:[" << src.width_ << "," << src.height_ << "]\n"
			<< "Video index:[" << src.index_ << "]\n"
			<< "Video fps:[" << src.fps_ << "]\n"
			<< endl;
		for (int k = 0; k < src.index_; k++) {
			bitwise_not(mask.frame_[k], mask.frame_[k]);
			process.BilinearInterpolation(src.frame_[k], mask.frame_[k]);
		}
		cout << "DONE!!\n" << endl;
		break;
	case 3:
		cout
			<< "Parameters reading\n"
			<< "Method: Diffusion inpainting (DI)\n"
			<< "Video size:[" << src.width_ << "," << src.height_ << "]\n"
			<< "Video index:[" << src.index_ << "]\n"
			<< "Video fps:[" << src.fps_ << "]\n"
			<< endl;
		for (int k = 0; k < src.index_; k++) {
			bitwise_not(mask.frame_[k], mask.frame_[k]);
			process.ImageInpainting(src.frame_[k], mask.frame_[k]);
		}
		cout << "DONE!!\n" << endl;
		break;
	case 4:
		cout
			<< "Parameters reading\n"
			<< "Method: Temporal replacement (TR)\n"
			<< "Video size:[" << src.width_ << "," << src.height_ << "]\n"
			<< "Video index:[" << src.index_ << "]\n"
			<< "Video fps:[" << src.fps_ << "]\n"
			<< endl;
		bitwise_not(mask.frame_[0], mask.frame_[0]);
		process.BilinearInterpolation(src.frame_[0], mask.frame_[0]);
		for (int k = 1; k < src.index_; k++) {
			bitwise_not(mask.frame_[k], mask.frame_[k]);
			process.TemporalReplacement(src.frame_[k], src.frame_[k - 1], mask.frame_[k]);
		}
		cout << "DONE!!\n" << endl;
		break;
	default:
		cout << "The method was not selected correctly!!\n" << endl;
	}
	cout << "Reconstruction method processing time:" << (float)(clock() - start) / CLOCKS_PER_SEC << " SEC.\n" << endl;

	// ========================================================================
	// Calculate PSNR and SSIM
	// ========================================================================
	cout << "Calculate PSNR and SSIM" << endl;
	process.EvaluationPSNR(src.frame_, src_clone, mask_clone, src.index_);
	process.EvaluationSSIM(src.frame_, src_clone, mask_clone, src.index_);

	char ch;
	cout << "Would you save image and video file? (Y/N)";
	cin >> ch;

	if (ch == 'Y' || ch == 'y')
		process.SaveImageAndVideo(src.frame_, src.width_, src.height_, src.index_, src.fps_);
	else if ((ch == 'N' || ch == 'n'))
		cout << "Program does not save result!!" << endl;
	else
		cout << "You have entered incorrectly!!" << endl;

	printf("END\n");

	if (src.frame_ != NULL) {
		delete[] src.frame_;
		src.frame_ = NULL;
	}
	if (mask.frame_ != NULL) {
		delete[] mask.frame_;
		mask.frame_ = NULL;
	}
	if (src_clone != NULL) {
		delete[] src_clone;
		src_clone = NULL;
	}
	if (mask_clone != NULL) {
		delete[] mask_clone;
		mask_clone = NULL;
	}

	return 0;
}