#include "ListView_Experiment.h"
#include "FSHelper.h"

#include <vector>
#include <iostream>


ListView_Experiment::ListView_Experiment(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::TreeView(cobject){
	m_refTreeModel = Gtk::ListStore::create(m_colums);
	set_model(m_refTreeModel);
	append_column("timestamp", m_colums.m_col_experimentTime);
	
	
	show_all_children();
}

ListView_Experiment::~ListView_Experiment(){
	
}

void ListView_Experiment::scanFolder(std::string path){
	std::vector<std::string> content = FSHelper::getFolderContent(path, true);
	
	m_refTreeModel->clear();
	
	for (std::vector<std::string>::iterator it = content.begin(); it != content.end(); it++){
		addExperiment(*it);
	}
}
void ListView_Experiment::addExperiment(std::string path){
	bool pathAlreadyInView = false;
	
	//search if path is already in view
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); it++){
		Gtk::TreeModel::Row row = *it;
		if (path.compare(Glib::ustring(row[m_colums.m_col_experimentPath])) == 0){
			pathAlreadyInView = true;
			break;
		}
	}
	
	if (!pathAlreadyInView){
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_colums.m_col_experimentPath] = path; // e.g. /home/pi/ProjectFolder/test/2019-05-26_13:10:46_test
		row[m_colums.m_col_experimentTime] = path.substr(path.find_last_of("/") + 1, 19); // e.g. 2019-05-26_13:10:46
	}
}

std::string ListView_Experiment::getSelectedExperimentPath(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return Glib::ustring(row[m_colums.m_col_experimentPath]);
	}
	return "";
}
std::string ListView_Experiment::getSelectedExperimentTime(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return Glib::ustring(row[m_colums.m_col_experimentTime]);
	}
	return "";
}