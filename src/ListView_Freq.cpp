#include "ListView_Freq.h"
#include "FSHelper.h"

#include <iostream>


ListView_Freq::ListView_Freq(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::TreeView(cobject){
	m_refTreeModel = Gtk::ListStore::create(m_colums);
	set_model(m_refTreeModel);
	append_column("frequncies", m_colums.m_col_freq_string);
	
	show_all_children();
}

ListView_Freq::~ListView_Freq(){
	
}

void ListView_Freq::addFreq(double freq){
	Gtk::TreeModel::Row row = *(m_refTreeModel->append());
	row[m_colums.m_col_freq] = freq;
	row[m_colums.m_col_freq_string] = FSHelper::formatDouble(freq);
}

double ListView_Freq::getSelectedFreq(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return row[m_colums.m_col_freq];
	}
	return 0;
}
int ListView_Freq::getSelectedFreq_rowNumber(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		return std::atoi(m_refTreeModel->get_path(it).to_string().c_str());
	}
	return -1;
}

void ListView_Freq::setFrequencies(std::vector<double> &freq){
	bool noChange = true;
	
	
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	
	//check sizes
	if ( children.size() != freq.size()){
		noChange = false;
	}else{ // sizes are the same
		int i = 0;
		for (Gtk::TreeModel::Children::iterator it = children.begin(); (it != children.end()) && noChange; it++){
			Gtk::TreeModel::Row row = *it;
			if (row[m_colums.m_col_freq] != freq.at(i++)){
				noChange = false;
			}
		}
	}
	
	if (!noChange){
		m_refTreeModel->clear();
		
		for (std::vector<double>::const_iterator cit = freq.cbegin(); cit != freq.cend(); cit++){
			addFreq(*cit);
		}
		std::cout << "change needed!" << std::endl;
	}else{
		std::cout << "no change needed!" << std::endl;
	}
}