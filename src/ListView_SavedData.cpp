#include "ListView_SavedData.h"
#include "FSHelper.h"

#include <vector>
#include <iostream>
#include <fstream>

ListView_SavedData::ListView_SavedData(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::TreeView(cobject){
	m_refTreeModel = Gtk::TreeStore::create(m_colums);
	set_model(m_refTreeModel);
	append_column("spectrum no", m_colums.m_col_spectrumNumber);
	append_column("spectrum type", m_colums.m_col_spectrumType_string);
	append_column("path", m_colums.m_col_spectrumPath);
	
	show_all_children();
}

ListView_SavedData::~ListView_SavedData(){
	
}

void ListView_SavedData::scanFolder(std::string path){
	std::vector<std::string> content = FSHelper::getFolderContent(path, false);
	
	m_refTreeModel->clear();
	
	for (std::vector<std::string>::iterator it = content.begin(); it != content.end(); it++){
		std::string filename = it->substr(it->find_last_of("/")+1);
		std::string filename_without_ending = filename.substr(0, filename.find_last_of("."));
		if (filename_without_ending.find("trans_imp_spectrum") == 0){ //spectrum data
			addTransImpSpectrum(*it, filename_without_ending);
		}else if (filename_without_ending.find("imp_spectrum") == 0){ //spectrum data
			addImpSpectrum(*it, filename_without_ending);
		}else if (filename_without_ending.find("opt_spectrum") == 0){ //spectrum data
			addSpectrum(*it, filename_without_ending);
		}
	}
}
bool ListView_SavedData::pathAlreadyInView(std::string path){
	//search if path is already in view
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); it++){
		Gtk::TreeModel::Row row = *it;
		if (path.compare(Glib::ustring(row[m_colums.m_col_spectrumPath])) == 0){
			return true;
		}
		
		for (Gtk::TreeModel::Children::iterator it2 = row.children().begin(); it2 != row.children().end(); it2++){
			Gtk::TreeModel::Row childrow = *it2;
			if (path.compare(Glib::ustring(childrow[m_colums.m_col_spectrumPath])) == 0){
				return true;
			}
		}
	}
	return false;
}
void ListView_SavedData::addSpectrum(std::string path, std::string filename){
	if (!pathAlreadyInView(path)){
		
		std::string number = filename.substr(std::string("opt_spectrum").length()); // remove "opt_spectrum"
		number = (number.empty()) ? "0" : number.substr(1);
		
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_colums.m_col_spectrumPath] = path;
		row[m_colums.m_col_spectrumNumber] = std::stoi(number);
		row[m_colums.m_col_spectrumType] = SPECTRUM_TYPE::SPECTRUM;
		row[m_colums.m_col_spectrumType_string] = "Spectrum";
	}
}
void ListView_SavedData::addImpSpectrum(std::string path, std::string filename){
	if (!pathAlreadyInView(path)){
		
		std::string number = filename.substr(std::string("imp_spectrum").length()); // remove "spectrum"
		number = (number.empty()) ? "0" : number.substr(1);
		
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_colums.m_col_spectrumPath] = path;
		row[m_colums.m_col_spectrumNumber] = std::stoi(number);
		row[m_colums.m_col_spectrumType] = SPECTRUM_TYPE::IMPEDANCE;
		row[m_colums.m_col_spectrumType_string] = "Impedance";
	}
}
void ListView_SavedData::addTransImpSpectrum(std::string path, std::string filename){
	if (!pathAlreadyInView(path)){
		
		std::string number = filename.substr(std::string("trans_imp_spectrum_").length()); // remove "trans_imp_spectrum_" -> get <trans_spec_no>_<spec_no>
		int trans_spectrum_no = std::stoi(number.substr(0, number.find("_")));
		std::string test = number.substr(number.find("_")+ 1);
		int spectrum_no = std::stoi(number.substr(number.find("_") + 1));
		
		
		Gtk::TreeModel::Row transientSpectrumRow;
		bool foundRow = false;
		// search if the trans imp spectrum already exists
		Gtk::TreeModel::Children children = m_refTreeModel->children();
		for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); it++){
			Gtk::TreeModel::Row row = *it;
			if (row[m_colums.m_col_spectrumType] == SPECTRUM_TYPE::TRANSIENT && row[m_colums.m_col_spectrumNumber] == trans_spectrum_no){ // found the spectrum
				transientSpectrumRow = row;
				foundRow = true;
				break;
			}
		}
		
		if (!foundRow){
			transientSpectrumRow = *(m_refTreeModel->append());
			transientSpectrumRow[m_colums.m_col_spectrumPath] = path;
			transientSpectrumRow[m_colums.m_col_spectrumNumber] = trans_spectrum_no;
			transientSpectrumRow[m_colums.m_col_spectrumType] = SPECTRUM_TYPE::TRANSIENT;
			transientSpectrumRow[m_colums.m_col_spectrumType_string] = "Transient";
		}
		
		Gtk::TreeModel::Row row = *(m_refTreeModel->append(transientSpectrumRow.children()));
		row[m_colums.m_col_spectrumPath] = path;
		row[m_colums.m_col_spectrumNumber] = spectrum_no;
		row[m_colums.m_col_spectrumType] = SPECTRUM_TYPE::IMPEDANCE;
		row[m_colums.m_col_spectrumType_string] = "Impedance";
	}
}
	
std::vector<std::string> ListView_SavedData::getSelectedTransSpectrumPaths(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	std::vector<std::string> paths;
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		if(static_cast<SPECTRUM_TYPE>((int) row[m_colums.m_col_spectrumType]) == SPECTRUM_TYPE::TRANSIENT){ //transient spectrum selected
			Gtk::TreeModel::Children children = row.children();
			
			//for all child rows add the path to the vector
			for (Gtk::TreeModel::Children::iterator it2 = children.begin(); it2 != children.end(); it2++){
				Gtk::TreeModel::Row child_row = *it2;
				std::string path = Glib::ustring(child_row[m_colums.m_col_spectrumPath]);
				
				paths.push_back(path);
			}
		}
	}
	return paths;
}
std::string ListView_SavedData::getSelectedSpectrumPath(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return Glib::ustring(row[m_colums.m_col_spectrumPath]);
	}
	return "";
}
ListView_SavedData::SPECTRUM_TYPE ListView_SavedData::getSelectedSpectrumType(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return static_cast<SPECTRUM_TYPE>((int) row[m_colums.m_col_spectrumType]);
	}
	return SPECTRUM_TYPE::ERROR;
}