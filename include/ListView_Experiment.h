#pragma once
/** 
 * @file ListView_Experiment.h
 * 
 * @author Nils Bosbach
 * @date 26.05.2019
 * @class ListView_Experiment
 * @brief 
 */

#include <gtkmm.h>
#include <vector>
#include <string>

class ListView_Experiment : public Gtk::TreeView{
	
public:
	ListView_Experiment(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ListView_Experiment();
	
	void scanFolder(std::string path);
	void addExperiment(std::string path);
	
	std::string getSelectedExperimentPath();
	std::string getSelectedExperimentTime();
	
protected:
	class ModelColumns : public Gtk::TreeModel::ColumnRecord{
	public:
		ModelColumns(){
			add(m_col_experimentTime);
			add(m_col_experimentPath);
		}
		Gtk::TreeModelColumn<Glib::ustring> m_col_experimentTime;
		Gtk::TreeModelColumn<Glib::ustring> m_col_experimentPath;
	};
	
	ModelColumns m_colums;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
	
};
