#pragma once
/** 
 * @file ListView_Project.h
 * 
 * @author Nils Bosbach
 * @date 26.05.2019
 * @class ListView_Project
 * @brief 
 */

#include <gtkmm.h>
#include <vector>
#include <string>

class ListView_Project : public Gtk::TreeView{
	
public:
	ListView_Project(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
	virtual ~ListView_Project();
	
	void scanFolder(std::string path);
	void addProject(std::string path);
	
	std::string getSelectedProjectPath();
	std::string getSelectedProjectName();
	
protected:
	class ModelColumns : public Gtk::TreeModel::ColumnRecord{
	public:
		ModelColumns(){
			add(m_col_projectName);
			add(m_col_projectPath);
		}
		Gtk::TreeModelColumn<Glib::ustring> m_col_projectName;
		Gtk::TreeModelColumn<Glib::ustring> m_col_projectPath;
	};
	
	ModelColumns m_colums;
	Glib::RefPtr<Gtk::ListStore> m_refTreeModel;
	
};

