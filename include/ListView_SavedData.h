#pragma once
/** 
 * @file ListView_SavedData.h
 * 
 * @author Nils Bosbach
 * @date 23.07.2019
 * @class ListView_SavedData
 * @brief 
 */
#include "DataP.h"

#include <gtkmm.h>
#include <vector>
#include <string>

class ListView_SavedData : public Gtk::TreeView{
	
public:
	enum SPECTRUM_TYPE {TRANSIENT, IMPEDANCE, SPECTRUM, ERROR};
	
	ListView_SavedData(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ListView_SavedData();
	
	void scanFolder(std::string path);
	
	std::string getSelectedSpectrumPath();
	std::vector<std::string> getSelectedTransSpectrumPaths();
	SPECTRUM_TYPE getSelectedSpectrumType();
	
protected:
	class ModelColumns : public Gtk::TreeModel::ColumnRecord{
	public:
		ModelColumns(){
			add(m_col_spectrumPath);
			add(m_col_spectrumNumber);
			add(m_col_spectrumType);
			add(m_col_spectrumType_string);
		}
		Gtk::TreeModelColumn<Glib::ustring> m_col_spectrumPath;
		Gtk::TreeModelColumn<int> m_col_spectrumNumber;
		Gtk::TreeModelColumn<int> m_col_spectrumType;
		Gtk::TreeModelColumn<Glib::ustring> m_col_spectrumType_string;
		
	};
	
	
	ModelColumns m_colums;
	Glib::RefPtr<Gtk::TreeStore> m_refTreeModel;
	
	bool pathAlreadyInView(std::string path);
	void addSpectrum(std::string path, std::string filename);
	void addImpSpectrum(std::string path, std::string filename);
	void addTransImpSpectrum(std::string path, std::string filename);
};

