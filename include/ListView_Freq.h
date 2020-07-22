#pragma once
/** 
 * @file ListView_Freq.h
 * 
 * @author Nils Bosbach
 * @date 05.06.2019
 * @class ListView_Freq
 * @brief 
 */

#include <gtkmm.h>
#include <string>

class ListView_Freq : public Gtk::TreeView{
	
public:
	ListView_Freq(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ListView_Freq();
	
	void addFreq(double freq);
	void setFrequencies(std::vector<double> &freq);
	double getSelectedFreq();
	int getSelectedFreq_rowNumber();
	
protected:
	class ModelColumns : public Gtk::TreeModel::ColumnRecord{
	public:
		ModelColumns(){
			add(m_col_freq);
			add(m_col_freq_string);
		}
		Gtk::TreeModelColumn<double> m_col_freq;
		Gtk::TreeModelColumn<Glib::ustring> m_col_freq_string;
	};
	
	ModelColumns m_colums;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
	
};

