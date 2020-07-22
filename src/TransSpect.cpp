#include "TransSpect.h"

#include <iostream>
#include <fstream>

TransSpect::TransSpect_ptr TransSpect::create(){
	TransSpect_ptr p = TransSpect_ptr(new TransSpect());
	p->this_ptr = p;
	return p;
}
TransSpect::TransSpect(){
	on_Spectrum_added = nullptr;
}
TransSpect::~TransSpect(){
	
}

void TransSpect::addSpectrum(Spectrum s){
	spectMtx.lock();
	
	if (transSpect.empty()){ // first spectrum 
		//read frequencies
		for (Spectrum::const_iterator cit = s.cbegin(); cit != s.cend(); cit++){
			frequencies.push_back((*cit)->getX());
		}
	}
	timeStamps.push_back(getCurrentTime());
	transSpect.push_back(s);
	
	unsigned int position = timeStamps.size() -1;
	std::chrono::duration<double> elapsed_seconds = timeStamps.at(position) - timeStamps.at(0);
	double timediff =  elapsed_seconds.count();
	spectMtx.unlock();
	
	if (on_Spectrum_added != nullptr) on_Spectrum_added(this_ptr.lock(), s, position, timediff);
}

TransSpect::TimePoint TransSpect::getCurrentTime(){
	return std::chrono::system_clock::now();
}

const std::vector<double> TransSpect::getFrequencies() const{
	return frequencies;
}

const TransSpect::Spectrum TransSpect::getTransSpect(unsigned int freqNo, TransSpect::X_VALUE x){
	Spectrum retSpect;
	spectMtx.lock();
	if (freqNo < frequencies.size() && transSpect.size() > 0){
		double x_val;
		double y_val_real;
		double y_val_imag;
		
		for (int i = 0; i < transSpect.size(); i++){
			y_val_real = transSpect.at(i).at(freqNo)->getY(DataP::COMPLEX_MODE::COMPLEX_REAL);
			y_val_imag = transSpect.at(i).at(freqNo)->getY(DataP::COMPLEX_MODE::COMPLEX_IMAG);
			
			switch (x){
				case X_VALUE::X_POINT:
					x_val = i;
					break;
				case X_VALUE::X_TIME_DIFF:
					std::chrono::duration<double> elapsed_seconds = timeStamps.at(i) - timeStamps.at(0);
					x_val = elapsed_seconds.count();
					break;
			}
			retSpect.push_back(std::make_shared<DataP>(x_val, y_val_real, y_val_imag));
		}
	}
	spectMtx.unlock();
	return retSpect;
}

void TransSpect::setOn_Spec_added_listener(void (*listener)(TransSpect::TransSpect_ptr ptr, TransSpect::Spectrum s, unsigned int position, double timediff)){
	on_Spectrum_added = listener;
}

void TransSpect::setBasePath(std::string path){
	basePath = path;
}

const std::string TransSpect::getBasePath() const{
	return basePath;
}

const TransSpect::Spectrum TransSpect::getLastSpectrum(){
	Spectrum s;
	
	spectMtx.lock();
	if (!transSpect.empty()){
		s = *(transSpect.rbegin());
	}
	spectMtx.unlock();
	
	return s;
}

const int TransSpect::getSpectrumCount() const{
	return transSpect.size();
}

const double TransSpect::getTimeDiff(int position) const{
	if (position >= 0 && position < timeStamps.size()){
		std::chrono::duration<double> elapsed_seconds = timeStamps.at(position) - timeStamps.at(0);
		double timediff =  elapsed_seconds.count();
	}
}

TransSpect::TransSpect_ptr TransSpect::loadTransImpSpectrum(std::vector<std::string> paths){
	TransSpect_ptr tSpect = create();
	TimePoint startTime = getCurrentTime();
	
	for(std::vector<std::string>::const_iterator cit = paths.cbegin(); cit != paths.cend(); cit++){
		Spectrum spectrum; // one spectrum will be stored here
		std::string path = *cit; // path of the file which contains the spectrum data
		int position = 0; // position of the spectrum in the transient spectrum
		double timediff = 0.0; // difference of the times between the first and the current spectrum
		std::ifstream spectrum_file(path); // variable to read the file
		
		//read spectrum
		if (spectrum_file.is_open()){
			std::string line;
			short frequency = -1, impedance_real = -1, impedance_imag = -1, impedance_abs = -1, impedance_phase = -1;
			short i = 0;
			int lineNo = 1;
		
			//overread possible header of transient impedance files
			while (std::getline(spectrum_file, line) && (line.find("position") == 0 || line.find("timediff") == 0)){
				lineNo++;
				
				if (line.find("position=") != std::string::npos){ // found position information
					position = std::stoi(line.substr(line.find("=") + 1));
				}
				if (line.find("timediff=") != std::string::npos){ // found position information
					timediff = std::stod(line.substr(line.find("=") + 1));
				}
			}
		
			
			if (!line.empty()){ // get header
			
			// read header
			std::string element;
			while (!line.empty()){ // read header
				if (line.find_first_of(";") == std::string::npos){ // last element in row
					element = line;
					line = "";
				}else{ // not the last element in row
					element = line.substr(0, line.find_first_of(";"));
					line = line.substr(line.find_first_of(";")+1);
				}
				if (element.compare("frequency") == 0){
					frequency = i;
					i++;
				}else if (element.compare("impedance_real") == 0){
					impedance_real = i;
					i++;
				}else if (element.compare("impedance_imag") == 0){
					impedance_imag = i;
					i++;
				}else if (element.compare("impedance_abs") == 0){
					impedance_abs = i;
					i++;
				}else if (element.compare("impedance_phase") == 0){
					impedance_phase = i;
					i++;
				}
			}
			
			if (frequency == -1){ // no frequency information found
				throw std::runtime_error("no freq information found in file" + path);
			}
			
			//read data
			std::vector<std::string> elementsOfLine;
			while(std::getline(spectrum_file, line)){ // for the rest of the document
				lineNo++;
				elementsOfLine.clear();
				
				while (!line.empty()){ // fill elementsOfLineVector
					if (line.find_first_of(";") == std::string::npos){ // last element in row
						elementsOfLine.push_back(line);
						line = "";
					}else{ // not the last element in row
						elementsOfLine.push_back(line.substr(0, line.find_first_of(";")));
						line = line.substr(line.find_first_of(";")+1);
					}
				}
				if (elementsOfLine.size() != i){ // no of elements does not fit the header
					throw std::runtime_error("line " + std::to_string(lineNo) + " parsing error - no. of elements dismatch header - " + path);
				}
				
				if ((impedance_real != -1) && (impedance_imag != -1)){ // got real and imag information
					DataP::DataP_ptr p = std::make_shared<DataP>(std::stod(elementsOfLine[frequency]), std::stod(elementsOfLine[impedance_real]), std::stod(elementsOfLine[impedance_imag]));
					spectrum.push_back(p);
				}else if ((impedance_phase != -1) && (impedance_abs != -1)){
					DataP::DataP_ptr p = std::make_shared<DataP>();
					p->setX(std::stod(elementsOfLine[frequency]));
					p->setY_AbsPhase(std::stod(elementsOfLine[impedance_abs]), std::stod(elementsOfLine[impedance_phase]), DataP::COMPLEX_MODE::COMPLEX_PHASE_RAD);
					spectrum.push_back(p);
				}else { ///@todo add other possible combinations
					throw std::runtime_error("data combination of real / imag / abs / phase not implemented");
				}
			}
			spectrum_file.close();
		}
	}
		
		
		//add spectrum to transient spectrum
		
		if (tSpect->transSpect.empty()){ // first spectrum 
			//read frequencies
			for (Spectrum::const_iterator cit = spectrum.cbegin(); cit != spectrum.cend(); cit++){
				tSpect->frequencies.push_back((*cit)->getX());
			}
			
			
		}
		
		tSpect->transSpect.push_back(spectrum);
		tSpect->timeStamps.push_back(startTime + std::chrono::milliseconds((long)(timediff * 1000)));
		std::cout << "loading spectrum " << path << std::endl;
	}
	
	return tSpect;
}