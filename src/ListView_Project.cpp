#include "ListView_Project.h"
#include "FSHelper.h"

#include <vector>
#include <iostream>


ListView_Project::ListView_Project(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder): Gtk::TreeView(cobject){
	m_refTreeModel = Gtk::ListStore::create(m_colums);
	set_model(m_refTreeModel);
	append_column("project name", m_colums.m_col_projectName);
	
	
	show_all_children();
}

ListView_Project::~ListView_Project(){
	
}

void ListView_Project::scanFolder(std::string path){
	std::vector<std::string> content = FSHelper::getFolderContent(path, true);
	
	m_refTreeModel->clear();
	
	for (std::vector<std::string>::iterator it = content.begin(); it != content.end(); it++){
		addProject(*it);
	}
}
void ListView_Project::addProject(std::string path){
	bool pathAlreadyInView = false;
	
	//search if path is already in view
	Gtk::TreeModel::Children children = m_refTreeModel->children();
	for (Gtk::TreeModel::Children::iterator it = children.begin(); it != children.end(); it++){
		Gtk::TreeModel::Row row = *it;
		if (path.compare(Glib::ustring(row[m_colums.m_col_projectPath])) == 0){
			pathAlreadyInView = true;
			break;
		}
	}
	
	if (!pathAlreadyInView){
		Gtk::TreeModel::Row row = *(m_refTreeModel->append());
		row[m_colums.m_col_projectPath] = path;
		row[m_colums.m_col_projectName] = path.substr(path.find_last_of("/") + 1);
	}
}

std::string ListView_Project::getSelectedProjectPath(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return Glib::ustring(row[m_colums.m_col_projectPath]);
	}
	return "";
}
std::string ListView_Project::getSelectedProjectName(){
	Gtk::TreeModel::iterator it = get_selection()->get_selected();
	
	if(it) { //something is selected
		Gtk::TreeModel::Row row = *it;
		return Glib::ustring(row[m_colums.m_col_projectName]);
	}
	return "";
}