//author: sebastian beyer 

#include <iostream>

#include <boost\filesystem.hpp>
#include <boost\format.hpp>
#include <boost\foreach.hpp>
#include <boost\lexical_cast.hpp>

#include <vector>
#include <string>
#include <memory>

#include <opencv2\opencv.hpp>

#include <conio.h>
#include <iostream>
#include <sstream>


#include <opencv2/ml/ml.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
namespace fs = boost::filesystem;

class ImageProcessing {
public:
	ImageProcessing() {

	}

	void setImage(const cv::Mat& image) {
		cv::imshow("Simulation Output", image);
		cv::Mat img = image.clone();
		double t1 = 0.3, t2 = 0.8;
		cv::blur(img, img, cv::Size(11, 11));
		cv::imshow("Processed", img);

		cv::waitKey(1);
	}
};

void draw(float ist, float soll) {

	if (ist >= 0)
	{
		ist = std::pow(ist, 4.0);
	}
	else
	{
		ist = -std::pow(ist, 4.0);
	}
	if (soll >= 0)
	{
		soll = std::pow(soll, 4.0);
	}
	else
	{
		soll = -std::pow(soll, 4.0);
	}


	int indexIst = (int)(ist * 10) + 10;
	int indexSoll = (int)(soll * 10) + 10;
	if ((indexIst > 20) || (indexSoll > 20))
	{
		std::cout << "error1";
	}
	if ((indexIst < 0) || (indexSoll < 0))
	{
		std::cout << "error2";
	}

	for (int i = 0; i <= 20; i++)
	{
		std::cout << "-";
		if (indexIst == i) {
			std::cout << "\bI";
		}
		if (indexSoll == i) {
			std::cout << "\bS";
		}

		if ((indexIst == indexSoll) && (indexIst == i)) {
			std::cout << "\b*";
		}
	}
	std::cout << "\n";
}

int main1() {

	std::string groundTruthDir = "C:\\Users\\Sebastian\\Desktop\\BAC\\GT-30-10-16";
	std::map<int /*frame number*/, cv::Mat> frames;
	std::map<int /*frame number*/, double> steeringAngles;
	

	//iterator over all files in directory
	fs::path targetDir(groundTruthDir.c_str()); 
	fs::directory_iterator it(targetDir), eod;
	std::string steeringAngleFilePath = "";

	int maxImages = 100;
	bool maxReached = false;
	BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
		if (fs::is_regular_file(p)) {
			if (p.has_extension()) {
				std::string extension = p.extension().string();
				if ((extension == ".bmp") && (maxReached == false)) {

					std::string filename = p.filename().string();
					std::cout << "\r";
					std::cout << "Loading image " << filename << "                  ";
					//remove extension
					size_t lastDotIndex = filename.find_last_of('.');
					filename = filename.substr(0, lastDotIndex);
					int frameNumber = boost::lexical_cast<int>(filename);

					/*if (frameNumber == maxImages)
					{
						maxReached = true;
					}*/

					cv::Mat image = cv::imread(p.string(), CV_LOAD_IMAGE_GRAYSCALE);
					frames.insert(std::pair<int, cv::Mat>(frameNumber, image));

				} else if (extension == ".csv") {
					steeringAngleFilePath = p.string();
				}
			}
		}
	}
	std::cout << "\n";
	//read file with steering angles
	if (steeringAngleFilePath == "") {
		return -1;
	}
	std::cout << "Loading Labels...\n";
	std::ifstream steeringAngleFile(steeringAngleFilePath);
	std::string line;
	int number = 0;
	while (std::getline(steeringAngleFile, line)) {
		number++;
		double steeringAngle = boost::lexical_cast<double>(line);
		//there is an error in the data, we have to flip the sign
		steeringAngle *= -1;
		steeringAngles.insert(std::pair<int, double>(number, steeringAngle));
	}

	int attributes_per_sample = 64 * 32; //size of one image
	int number_of_training_samples = maxImages; //number of images
	int number_of_classes = 1; //size of output vector for steering direction
	int hidden_layer_size = 15; //size of the hidden layer of neurons

	cv::Mat training_data = cv::Mat(number_of_training_samples, attributes_per_sample, CV_32FC1);
	cv::Mat training_classifications = cv::Mat(number_of_training_samples, number_of_classes, CV_32FC1, 0.0);

	//for (int i = 0; i < number_of_training_samples; i++) {
	//	int classNumber = (int)steeringAngles[i + 1];
	//	classNumber = std::min(classNumber, ((number_of_classes -1)/2));
	//	classNumber = std::max(classNumber, -((number_of_classes - 1) / 2));
	//	classNumber += ((number_of_classes - 1) / 2);
	//	training_classifications.at<float>(i, classNumber) = 1.0;

	//	cv::Rect region(0, i, number_of_classes, 1);
	//	cv::GaussianBlur(training_classifications(region), training_classifications(region), cv::Size(31, 1), 2.0, 2.0, cv::BORDER_CONSTANT);
	//}
	float maxAngle = 15.3;
	float minAngle = -25.5;
	for (int i = 0; i < number_of_training_samples; i++) {
		float angle = steeringAngles[i + 1];
		//normalize
		if (angle >= 0) {
			angle /= maxAngle;
		} else {
			angle /= minAngle;
			angle *= -1;
		}
		//4th root
		if (angle >= 0) {
			angle = std::pow(angle, 0.25);
		} else {
			angle = -std::pow(-angle, 0.25);
		}

		training_classifications.at<float>(i, 0) = angle;

		/*
		cv::Rect region(0, i, number_of_classes, 1);
		cv::GaussianBlur(training_classifications(region), training_classifications(region), cv::Size(31, 1), 2.0, 2.0, cv::BORDER_CONSTANT);
		*/
	}
	
	//training_classifications *= 5; //normalize the image to 1;

	//for (int i = 0; i < number_of_training_samples; i++) {
	//	int classNumber;
	//	if (steeringAngles[i] > 3.0) {
	//		classNumber = 2;
	//	} else if (steeringAngles[i] < -3.0) {
	//		classNumber = 0;
	//	} else {
	//		classNumber = 1;
	//	}
	//	training_classifications.at<float>(i, classNumber) = 1.0;
	//}

	for (int i = 0; i < number_of_training_samples; i++) {
		std::cout << "\rProcessing image #" << i+1 << " of " << number_of_training_samples << "       ";
		cv::Mat imageFloat;
		cv::Mat currentImage = frames[i+1];
		cv::Mat equalizedImage;
		cv::equalizeHist(currentImage, equalizedImage);
		equalizedImage.convertTo(imageFloat, CV_32FC1, (1 / 128.0), -1.0);
		cv::Mat row = imageFloat.reshape(1, 1);

		row.copyTo(training_data(cv::Rect(0, i, row.cols, row.rows)));
	}
	std::cout << "\n";
	std::vector<int> layerSizes = { attributes_per_sample, 100, hidden_layer_size, 3, number_of_classes };


	cv::Ptr<cv::ml::ANN_MLP> ann = cv::ml::ANN_MLP::create();

	ann->setLayerSizes(layerSizes);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM);
	cv::Ptr<cv::ml::TrainData> trainData = cv::ml::TrainData::create(training_data, cv::ml::SampleTypes::ROW_SAMPLE, training_classifications);
	trainData->setTrainTestSplitRatio(0.95);
	cv::TermCriteria termCriteria;
	
	//termCriteria.maxCount = 400;
	//termCriteria.epsilon = 0.00001f;
	termCriteria.maxCount = 50000;
	termCriteria.epsilon = 0.00001f;
	termCriteria.type = cv::TermCriteria::EPS;
	ann->setTermCriteria(termCriteria);
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
	ann->setBackpropMomentumScale(0.0001);
	ann->setBackpropWeightScale(0.02);

	std::cout << "Training neural network..." << std::endl;
	int doneIterations = ann->train(trainData);
	ann->save("C:\\Temp\\badlyTrainedNetwork.ann");
	cv::Mat useless;
	float uselessError = ann->calcError(trainData, true, useless);
	cv::Mat out;
	cv::Mat idx;
	std::cout << "Training done with " << doneIterations << " iterations. Error = " << uselessError << std::endl;
	cv::Mat1i testSamples = trainData->getTestSampleIdx();
	cv::Mat samples = trainData->getSamples();
	cv::Mat results;
	std::cout << "Running complete training set again..." << std::endl;
	ann->predict(samples, results);
	cv::namedWindow("image", cv::WINDOW_FREERATIO);
	double sumOfSquares = 0.0;
	std::cout << "Done. Processing results.." << std::endl;
	for (int i = 0; i < testSamples.cols; i++) {
		int sampleNr = testSamples(cv::Point(i, 0));
		int frameNr = sampleNr + 1;
		float result = results.at<float>(cv::Point(0, sampleNr));
		float soll = training_classifications.at<float>(cv::Point(0, sampleNr));
		//std::cout << boost::format("%6.2f") % soll << " - " << boost::format("%6.2f") % result << " = " << boost::format("%6.2f") % (soll-result) << "\n" ;
		sumOfSquares += ((soll - result)*(soll - result));
		cv::Mat image = frames[frameNr];
		//cv::imshow("image", image);
		//cv::waitKey(1);
	}
	double rms = std::sqrt(sumOfSquares / testSamples.cols);
	std::cout << "Total Error on Test Set (RMS): " << rms;

	for (int i = 0; i < number_of_training_samples; i++) {
		int sampleNr = i;
		int frameNr = sampleNr + 1;
		float result = results.at<float>(cv::Point(0, sampleNr));
		float soll = training_classifications.at<float>(cv::Point(0, sampleNr));
		std::cout << boost::format("%6.2f") % soll << " - " << boost::format("%6.2f") % result << " = " << boost::format("%6.2f") % (soll - result) << "\n";
		sumOfSquares += ((soll - result)*(soll - result));

		draw(result, soll);
		
		cv::Mat image = frames[frameNr];
		cv::imshow("image", image);
		cv::waitKey(-1);
	}
	rms = std::sqrt(sumOfSquares / testSamples.cols);
	std::cout << "Total Error on Test Set (RMS): " << rms;

	std::getchar();
	/*try {
		float error = ann->calcError(trainData, true, out);
	
		std::cout << error << std::endl;
	} catch (std::exception e) {
		std::string w = e.what();
		std::cout << w << std::endl;
	}
	
	
	for (int i = 0; i < idx.cols; i++) {
		int frameNr = idx.at<int>(0, i);
		float result = out.at<float>(i, 0);
		cv::Mat image = frames[frameNr + 1];
		std::cout << "\r";
		std::cout << result;
		cv::imshow("image", image);
		cv::waitKey(-1);
	}*/

	/*for (int i = 0; i < idx.cols; i++) {
		int frameNr = idx.at<int>(0, i);
		int result = out.at<float>(i, 0);
		cv::Mat image = frames[frameNr + 1];
		std::cout << "\r";
		for (int c = 0; c < result; c++)
		{
			std::cout << "-";
		}
		std::cout << "*";
		for (int c = result+1; c < number_of_classes; c++)
		{
			std::cout << "-";
		}
		cv::imshow("image", image);
		cv::waitKey(-1);
	}*/
	
	return 0;
}

int main() {

	std::string groundTruthDir = "C:\\ptemp\\trainingData9";
	std::map<int /*frame number*/, cv::Mat> frames;
	std::map<int /*frame number*/, double> steeringAngles;


	//iterator over all files in directory
	fs::path targetDir(groundTruthDir.c_str());
	fs::directory_iterator it(targetDir), eod;
	std::string steeringAngleFilePath = "";

	int maxImages = 12530;
	bool maxReached = false;
	BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod)) {
		if (fs::is_regular_file(p)) {
			if (p.has_extension()) {
				std::string extension = p.extension().string();
				if (((extension == ".bmp")  || (extension == ".png"))&& (maxReached == false)) {

					std::string filename = p.filename().string();
					std::cout << "\r";
					std::cout << "Loading image " << filename << "                  ";
					//remove extension
					size_t lastDotIndex = filename.find_last_of('.');
					filename = filename.substr(0, lastDotIndex);
					std::string numberStr = filename.substr(0, 5);
					std::string angleStr = filename.substr(6, 9);
					int frameNumber = boost::lexical_cast<int>(numberStr);
					double angle = 0.0;
					try {
						angle = boost::lexical_cast<double>(angleStr);
					} catch (boost::bad_lexical_cast blc) {
						std::cout << "bad lexical cast on string " << angleStr  << ", frame number "<< frameNumber << std::endl;
					}
					/*if (frameNumber == maxImages)
					{
					maxReached = true;
					}*/

					cv::Mat image = cv::imread(p.string(), CV_LOAD_IMAGE_GRAYSCALE);
					frames.insert(std::pair<int, cv::Mat>(frameNumber, image));
					steeringAngles.insert(std::pair<int, double>(frameNumber, angle));

				}
			}
		}
	}
	std::cout << "\n";
	//read file with steering angles
	//if (steeringAngleFilePath == "") {
	//	return -1;
	//}


	int attributes_per_sample = 64 * 32; //size of one image
	int number_of_training_samples = maxImages; //number of images
	int number_of_classes = 1; //size of output vector for steering direction
	int hidden_layer_size = 15; //size of the hidden layer of neurons

	cv::Mat training_data = cv::Mat(number_of_training_samples, attributes_per_sample, CV_32FC1);
	cv::Mat training_classifications = cv::Mat(number_of_training_samples, number_of_classes, CV_32FC1, 0.0);

	for (int i = 0; i < number_of_training_samples; i++) {
		training_classifications.at<float>(i, 0) = steeringAngles[i];
	}

	//for (int i = 0; i < number_of_training_samples; i++) {
	//	int classNumber = (int)steeringAngles[i + 1];
	//	classNumber = std::min(classNumber, ((number_of_classes -1)/2));
	//	classNumber = std::max(classNumber, -((number_of_classes - 1) / 2));
	//	classNumber += ((number_of_classes - 1) / 2);
	//	training_classifications.at<float>(i, classNumber) = 1.0;

	//	cv::Rect region(0, i, number_of_classes, 1);
	//	cv::GaussianBlur(training_classifications(region), training_classifications(region), cv::Size(31, 1), 2.0, 2.0, cv::BORDER_CONSTANT);
	//}
	//float maxAngle = 1.0;
	//float minAngle = -1.0;
	//for (int i = 0; i < number_of_training_samples; i++) {
	//	float angle = steeringAngles[i + 1];
	//	//normalize
	//	if (angle >= 0) {
	//		angle /= maxAngle;
	//	}
	//	else {
	//		angle /= minAngle;
	//		angle *= -1;
	//	}
	//	//4th root
	//	if (angle >= 0) {
	//		angle = std::pow(angle, 0.25);
	//	}
	//	else {
	//		angle = -std::pow(-angle, 0.25);
	//	}
	//
	//	training_classifications.at<float>(i, 0) = angle;
	//
	//	/*
	//	cv::Rect region(0, i, number_of_classes, 1);
	//	cv::GaussianBlur(training_classifications(region), training_classifications(region), cv::Size(31, 1), 2.0, 2.0, cv::BORDER_CONSTANT);
	//	*/
	//}

	//training_classifications *= 5; //normalize the image to 1;

	//for (int i = 0; i < number_of_training_samples; i++) {
	//	int classNumber;
	//	if (steeringAngles[i] > 3.0) {
	//		classNumber = 2;
	//	} else if (steeringAngles[i] < -3.0) {
	//		classNumber = 0;
	//	} else {
	//		classNumber = 1;
	//	}
	//	training_classifications.at<float>(i, classNumber) = 1.0;
	//}

	for (int i = 0; i < number_of_training_samples; i++) {
		std::cout << "\rProcessing image #" << i + 1 << " of " << number_of_training_samples << "       ";
		cv::Mat imageFloat;
		cv::Mat currentImage = frames[i + 1];
		
		cv::Mat bin;
		int windowSize = 501;
		double C = -100;
		cv::adaptiveThreshold(currentImage, bin, 255.0, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, windowSize, C);
		cv::Mat resizedBin;
		cv::resize(bin, resizedBin, cv::Size(64, 32));
		resizedBin.convertTo(imageFloat, CV_32FC1, (1 / 128.0), -1.0);
		cv::Mat row = imageFloat.reshape(1, 1);

		row.copyTo(training_data(cv::Rect(0, i, row.cols, row.rows)));
	}
	std::cout << "\n";
	std::vector<int> layerSizes = { attributes_per_sample, 100, hidden_layer_size, 3, number_of_classes };


	cv::Ptr<cv::ml::ANN_MLP> ann = cv::ml::ANN_MLP::create();

	ann->setLayerSizes(layerSizes);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM);
	cv::Ptr<cv::ml::TrainData> trainData = cv::ml::TrainData::create(training_data, cv::ml::SampleTypes::ROW_SAMPLE, training_classifications);
	trainData->setTrainTestSplitRatio(0.95);
	cv::TermCriteria termCriteria;

	//termCriteria.maxCount = 400;
	//termCriteria.epsilon = 0.00001f;
	termCriteria.maxCount = 50000;
	termCriteria.epsilon = 0.00001f;
	termCriteria.type = cv::TermCriteria::EPS;
	ann->setTermCriteria(termCriteria);
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP);
	ann->setBackpropMomentumScale(0.0001);
	ann->setBackpropWeightScale(0.02);

	std::cout << "Training neural network..." << std::endl;
	int doneIterations = ann->train(trainData);
	ann->save("C:\\Temp\\anotherNetwork_justATest.ann");
	cv::Mat useless;
	float uselessError = ann->calcError(trainData, true, useless);
	cv::Mat out;
	cv::Mat idx;
	std::cout << "Training done with " << doneIterations << " iterations. Error = " << uselessError << std::endl;
	cv::Mat1i testSamples = trainData->getTestSampleIdx();
	cv::Mat samples = trainData->getSamples();
	cv::Mat results;
	std::cout << "Running complete training set again..." << std::endl;
	ann->predict(samples, results);
	cv::namedWindow("image", cv::WINDOW_FREERATIO);
	double sumOfSquares = 0.0;
	std::cout << "Done. Processing results.." << std::endl;

	std::ofstream errorfile("C:\\Temp\\errorfile_good_network2.txt");
	errorfile << "sampleNumber;soll-result" << std::endl;

	for (int i = 0; i < testSamples.cols; i++) {
		int sampleNr = testSamples(cv::Point(i, 0));
		int frameNr = sampleNr + 1;
		float result = results.at<float>(cv::Point(0, sampleNr));
		float soll = training_classifications.at<float>(cv::Point(0, sampleNr));
		//std::cout << boost::format("%6.2f") % soll << " - " << boost::format("%6.2f") % result << " = " << boost::format("%6.2f") % (soll-result) << "\n" ;
		sumOfSquares += ((soll - result)*(soll - result));
		cv::Mat image = frames[frameNr];
		errorfile << i << ";" << soll - result << std::endl;
		//cv::imshow("image", image);
		//cv::waitKey(1);
	}
	double rms = std::sqrt(sumOfSquares / testSamples.cols);
	std::cout << "Total Error on Test Set (RMS): " << rms << std::endl;
	errorfile.close();

	std::getchar();
	for (int i = 0; i < number_of_training_samples; i++) {
		int sampleNr = i;
		int frameNr = sampleNr + 1;
		float result = results.at<float>(cv::Point(0, sampleNr));
		float soll = training_classifications.at<float>(cv::Point(0, sampleNr));
		std::cout << boost::format("%6.2f") % soll << " - " << boost::format("%6.2f") % result << " = " << boost::format("%6.2f") % (soll - result) << "\n";
		sumOfSquares += ((soll - result)*(soll - result));

		
		errorfile.flush();
		draw(result, soll);

		cv::Mat image = frames[frameNr];
		cv::imshow("image", image);
		//cv::waitKey(1);
	}
	
	rms = std::sqrt(sumOfSquares / testSamples.cols);
	std::cout << "Total Error on Training Set (RMS): " << rms;

	std::getchar();
	/*try {
	float error = ann->calcError(trainData, true, out);

	std::cout << error << std::endl;
	} catch (std::exception e) {
	std::string w = e.what();
	std::cout << w << std::endl;
	}


	for (int i = 0; i < idx.cols; i++) {
	int frameNr = idx.at<int>(0, i);
	float result = out.at<float>(i, 0);
	cv::Mat image = frames[frameNr + 1];
	std::cout << "\r";
	std::cout << result;
	cv::imshow("image", image);
	cv::waitKey(-1);
	}*/

	/*for (int i = 0; i < idx.cols; i++) {
	int frameNr = idx.at<int>(0, i);
	int result = out.at<float>(i, 0);
	cv::Mat image = frames[frameNr + 1];
	std::cout << "\r";
	for (int c = 0; c < result; c++)
	{
	std::cout << "-";
	}
	std::cout << "*";
	for (int c = result+1; c < number_of_classes; c++)
	{
	std::cout << "-";
	}
	cv::imshow("image", image);
	cv::waitKey(-1);
	}*/

	return 0;
}


int main2() {
	
	//load the distorted image that shows a street and a calibration-chessboard "C:\ptemp\trainingData4\00268_+0.010000000.png"
	cv::Mat distortedImage = cv::imread("C:\\ptemp\\trainingData4\\00268_+0.010000000.png", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat distortedImage = cv::imread("C:\\Users\\Sebastian\\Desktop\\BAC\\small_calib_image.png", CV_LOAD_IMAGE_GRAYSCALE);
	std::vector<cv::Point2d> distortedPoints;
	cv::Size chessboardSize = cv::Size(7, 7);
	//bool patternFound = cv::findChessboardCorners(distortedImage, chessboardSize, distortedPoints, CV_CALIB_CB_ADAPTIVE_THRESH);
	//cv::Mat colorImage;
	//cv::cvtColor(distortedImage, colorImage, CV_GRAY2BGR);
	////cv::drawChessboardCorners(colorImage, chessboardSize, distortedPoints, true);
	//for (int i = 0; i < distortedPoints.size(); i++) {
	//	double weight = (double)i / (double)(distortedPoints.size()-1);
	//	cv::circle(colorImage, distortedPoints[i], 5, cv::Scalar(0, weight*255, (1-weight)*255));
	//}

	distortedPoints.push_back(cv::Point( 53, 120));
	distortedPoints.push_back(cv::Point( 88, 56));
	distortedPoints.push_back(cv::Point(170, 56));
	distortedPoints.push_back(cv::Point(205, 120));


	std::vector<cv::Point2d> undistoredPoints;
	undistoredPoints.push_back(cv::Point(400, 800));
	undistoredPoints.push_back(cv::Point(400, 600));
	undistoredPoints.push_back(cv::Point(600, 600));
	undistoredPoints.push_back(cv::Point(600, 800));


	
	//
	//for (int x = 1; x <= 7; x++) {
	//	for (int y = 7; y >= 1; y--) {
	//		undistoredPoints.push_back(cv::Point((x*100)+10000, (y*100)+19000));
	//	}
	//}

	//for (int i = 0; i < undistoredPoints.size(); i++) {
	//	double weight = (double)i / (double)(undistoredPoints.size() - 1);
	//	cv::circle(colorImage, undistoredPoints[i], 5, cv::Scalar(0, weight * 255, (1 - weight) * 255), -1);
	//}


	cv::Mat H;
	cv::Mat ortoImage;
	try {
		//T = cv::getPerspectiveTransform(distortedPoints, undistoredPoints);
		cv::Mat outMask;
		H = cv::findHomography(distortedPoints, undistoredPoints, outMask);

		cv::FileStorage fs("C:\\ptemp\\transform.yml", cv::FileStorage::WRITE);

		time_t rawtime; time(&rawtime);
		fs << "calibrationDate" << asctime(localtime(&rawtime));
		fs << "transformationMatrix" << H;
		fs.release();
		cv::warpPerspective(distortedImage, ortoImage, H, cv::Size(1000,1000), CV_INTER_LANCZOS4);
		cv::Mat img = cv::imread("C:\\ptemp\\trainingData3\\00049_+0.100000000.png", CV_LOAD_IMAGE_GRAYSCALE);
		cv::warpPerspective(img, ortoImage, H, cv::Size(1000, 1000));
		//cv::warpAffine(distortedImage, ortoImage, H.inv(), distortedImage.size());
	}
	catch (cv::Exception e) {
		std::string what = e.what();
		std::cout << what;
	}


	return 0;
}