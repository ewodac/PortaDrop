#pragma once
/** 
 * @file TreeView_Recipe.h
 * 
 * @class TreeView_Recipe
 * @author Nils Bosbach
 * @date 19.04.2019
 * @brief uses a Gtk::TreeView to visualize a Recipe / Tasks of a Recipe
 * @see Recipe
 * @see Task
 */
#include "Recipe.h"

#include <gtkmm.h>
#include <vector>

class TreeView_Recipe{
	
public:
	/**
	 * @param tv the Gtk:TreeView object which will be used to visualize the Recipe
	 * @param mainWindow Gtk:Window, mother object of the Gtk::TreeView
	 */
	TreeView_Recipe(Gtk::TreeView* tv, Gtk::Window* mainWindow);
	~TreeView_Recipe();
	
	/**
	 * @brief set the tasks which should be displayed in the view
	 * @param tasks list of tasks
	 */
	void setTasks(std::vector<Task::Task_ptr> tasks);
	
	/**
	 * @brief adds a Recipe to the recipe list and displays it in the gui
	 * @param recipe the Recipe which should be added
	 */
	void addTask(Task::Task_ptr task);
	
	/**
	 * @brief returns the selected recipe
	 * @return  the selected recipe
	 */
	Task::Task_ptr getSelectedTask() const;
	
	/**
	 * @brief get the tasks displayed in the gui
	 * @return a std::vector of the tasks same ordered as shown in the gui
	 */
	std::vector<Task::Task_ptr> getTasks() const;
	
	/**
	 * @brief removes the selected row
	 * removes row from gui and deletes recipe from recipes if the selected row was the only reference of the recipe
	 */
	void deleteSelectedRow();
	
	/**
	 * @brief enables / disables the treeView
	 * @param value if true, the TreeView will be enabled
	 */
	void set_TreeView_sensitive(bool value);
	
	/**
	 * @brief creates a recipe containing all tasks of the TreeView
	 * @param name the name of the created recipe
	 * @param comment the comment of the created recipe
	 * @return a pointer to the created Recipe
	 */
	Recipe::Recipe_ptr getAllTasksAsOneRecipe(std::string name = "unset", std::string comment = "") const;
	
	/**
	 * @brief removes all tasks from TreeView
	 */
	void clear();
	
protected:
	/**
	 * @class TreeModel_Recipe
	 * @brief TreeModel which is used for the Gtk::TreeView
	 * handles the data of the TreeView
	 */
	class TreeModel_Recipe : public Gtk::TreeStore{
	public:
		/**
		 * @class ModelColumns
		 * @brief defines which columns are used in the Gtk::TreeView
		 */
		class ModelColumns : public Gtk::TreeModel::ColumnRecord{
		public:
			/**
			 * initializes the columns
			 */
			ModelColumns(){
				add(m_col_id);
				add(m_col_name);
				add(m_col_type);
//				add(m_col_active);
				add(m_draggable);
			}
			Gtk::TreeModelColumn<int> m_col_id;
			Gtk::TreeModelColumn<Glib::ustring> m_col_name;
			Gtk::TreeModelColumn<Glib::ustring> m_col_type;
//			Gtk::TreeModelColumn<bool> m_col_active;
			Gtk::TreeModelColumn<bool> m_draggable;
		};
		
		ModelColumns columns;
		
		/**
		 * This function is needed to call the protected constructor of TreeModel_Recipe()
		 * @return Glib::RefPtr<TreeModel_Recipe> of a new TreeModel_Recipe object
		 */
		static Glib::RefPtr<TreeModel_Recipe> create(){return Glib::RefPtr<TreeModel_Recipe>(new TreeModel_Recipe());}
		
	protected:
		TreeModel_Recipe();
		
		/**
		 * @brief decides if a row is draggable or not
		 * @param path path to the row which should be draggable or not
		 * @return true, if row should be draggable; false if not
		 * This function needs to override the function declared in the mother class to determine whether
		 * a row should be draggable or not. It is used to just enable the dragging of rows which m_draggable
		 * attribute is set to true (top level rows).
		 */
		bool row_draggable_vfunc(const Gtk::TreeModel::Path& path) const override;
		
		/**
		 * @brief decides if a dragged row can be inserted at the current path
		 * @param dest path to the place where the row should be inserted
		 * @param selection_data provides information about the row which should be inserted
		 * @return true, if the row can be inserted at the current position; false if not
		 * This function needs to override the function declared in the mother class to determine whether
		 * a row can be inserted at a specific position. It is used to just enable dropping a row a the top
		 * level.
		 */
		bool row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const override;
	};
	void on_listView_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selectionData, guint info, guint time);
	void on_listView_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);
		
	/**
	 * @brief determines whether a row can be selected or not 
	 * @param model the used treeModel
	 * @param path path of the row which the user wants to select
	 * @return true if path belongs to a top layer row; false otherwise
	 */
	bool select_function(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool);
	std::vector<Gtk::TargetEntry> listTargets;
	
private:
	///the gui element of the treeView
	Gtk::TreeView *treeView_recipe;
	///handles the selection of the treeView
	Glib::RefPtr<Gtk::TreeSelection> treeView_recipe_selection;
	///handles the data of the treeView
	Glib::RefPtr<TreeModel_Recipe> treeModel;
	/**
	 * recipes which should be displayed in the TreeView
	 * every recipe can at maximum be once in the list (but more than once in the treeView)
	 * the vector should be sorted in acending order (ID)
	 */
	std::vector<Task::Task_ptr> tasks;
	
	/**
	 * @brief adds an existing Task t to the TreeView
	 * @param t the Task which should be added
	 */
	void addTaskToView(Task::Task_ptr t);
	
	/**
	 * @brief adds all Tasks / child recipes of a Recipe as childrows to an exisiting row
	 * @see addRecipeToView
	 * This function is used by addRecipeToView and implements a recusive algorithm to add all child
	 * recipes or tasks of the Recipe to the TreeView
	 */
	void addChildRecipesToView(Recipe::Recipe_ptr recipe, Gtk::TreeModel::Row* motherRow);

};

