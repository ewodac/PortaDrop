#pragma once
/**
 * @file TransSpect.h
 * 
 * @class TransSpect
 * @author Nils Bosbach
 * @date 05.06.2019
 * @brief class to store a transient spectrum. A listener can be added to call a specified function each time a spectrum has been added to
 * the transient spectrum. The captured spectrums and timestamps are stored and a transient spectrum of one frequency can be created
 */
#include "DataP.h"

#include <vector>
#include <chrono>
#include <mutex>
#include <memory>
#include <string>


class TransSpect{
public:
	typedef std::vector<DataP::DataP_ptr> Spectrum;
	typedef std::shared_ptr<TransSpect> TransSpect_ptr;
	
	enum X_VALUE {X_TIME, X_TIME_DIFF, X_POINT};
	
	/**
	 * @brief creates a new TransSpect object - use this method as named constructor
	 * 
	 * The constructor of TransSpect is private to ensure the object is deleted at the right time. The on_Spectrum_added listener has
	 * a TransSpect_ptr parameter. To ensure the objec is (only) deleted when the last reference to this object is deleted, it is neccessary
	 * that only shared_ptr references of the object exist (and not a variable contianing the object itself). The class has a weak_ptr 
	 * reference to itself, which is locked (transformed in a shared_ptr reference) when the listener is called.
	 */
	static TransSpect_ptr create();
	virtual ~TransSpect();
	
	/**
	 * @brief adds a new Spectrum to the transient spectrum. The time stamp of the calltime of the function is saved and the on_Spectrum_added
	 * listener is called (if set).
	 * @param s the spectrum which should be added
	 */
	void addSpectrum(Spectrum s);
	
	/**
	 * @brief returns a vector containing all frequencies of the captured spectrums. Assuming all spectrums contain the same frequencies
	 * (this is not checked), technically the freuqencies of the first captured spectrum are returned
	 * @return the frequencies of the spectrums
	 */
	const std::vector<double> getFrequencies() const;
	
	/**
	 * @brief creates a transient spectrum for one frequency
	 * @param freqNo number of the frequency which should be used for the transient spectrum (index of frequency in getFrequencies() vector)
	 * @param x defines what should be used for the x values (time or measurement number)
	 */
	const Spectrum getTransSpect(unsigned int freqNo, X_VALUE x = X_VALUE::X_POINT);
	
	/**
	 * @brief links the method which is called each time a spectrum has been added
	 * @param listener the method which should be called
	 */
	void setOn_Spec_added_listener(void (*listener)(TransSpect_ptr ptr, Spectrum s, unsigned int position, double timediff));
	
	/**
	 * @brief set the path where the spectrums are saved (base path because _<index>.xml will be added to each file)
	 * @param path the base path (where just "_<index>.xml" needs to be added)
	 * @see ExperimentData
	 * 
	 * this function is called by ExperimentData to store the calculated base path
	 */
	void setBasePath(std::string path);
	
	/**
	 * @brief get the value of the basepath variable (which has been set by setBasePath)
	 * @return the base path (where just "_<index>.xml" needs to be added for each spectrum)
	 */
	const std::string getBasePath() const;
	
	/**
	 * @brief get the last added spectrum (last element in transSpect)
	 * @return the last added spectrum
	 */
	const Spectrum getLastSpectrum();
	
	/**
	 * @brief get the number of captured spectrums
	 * @return number of captured spectrums
	 */
	const int getSpectrumCount() const;
	
	/**
	 * @brief get the elapsed seconds between the first captured spectrum and the spectrum at a specific position
	 * @param position position of the specific spectrum
	 * @return timediff between the first captured spectrum an the one at the specified position
	 */
	const double getTimeDiff(int position) const;
	
	/// TransImpTask stores the progress in this variable (e.g. 0.5 if half of the spectrums are captured)
	double progress = 0.0;
	
	/**
	 * @brief load a saved transient spectrum
	 * @param paths list of the paths of the files that belong to the transient spectrum
	 * @return smart pointer to the loaded spectrum
	 */
	static TransSpect_ptr loadTransImpSpectrum(std::vector<std::string> paths);
	
private:
	typedef std::chrono::system_clock::time_point TimePoint;
	
	TransSpect();
	std::vector<Spectrum> transSpect;
	std::vector<TimePoint> timeStamps;
	std::vector<double> frequencies;
	std::mutex spectMtx;
	std::string basePath;
	
	void (*on_Spectrum_added) (TransSpect_ptr ptr, Spectrum s, unsigned int position, double timediff);
	
	static inline TimePoint getCurrentTime();
	std::weak_ptr<TransSpect> this_ptr;
	
};
