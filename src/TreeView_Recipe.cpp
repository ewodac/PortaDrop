#include "TreeView_Recipe.h"

#include <iostream>
#include <algorithm>

TreeView_Recipe::TreeView_Recipe(Gtk::TreeView* tv, Gtk::Window* mainWindow){
	treeView_recipe = tv;
	
	treeModel = TreeModel_Recipe::create();
	treeView_recipe->set_model(treeModel);
	treeView_recipe->set_reorderable(); //enable drag&drop
	
	//add columns
	treeView_recipe->append_column("ID", treeModel->columns.m_col_id);
	treeView_recipe->append_column("name", treeModel->columns.m_col_name);
	treeView_recipe->append_column("type", treeModel->columns.m_col_type);
	
	//allow selection just at the top layer
	treeView_recipe_selection = treeView_recipe->get_selection();
	treeView_recipe_selection->set_select_function( sigc::mem_fun(*this, &TreeView_Recipe::select_function));
	
}

TreeView_Recipe::~TreeView_Recipe(){
}

TreeView_Recipe::TreeModel_Recipe::TreeModel_Recipe(){
	set_column_types(columns);
}

bool TreeView_Recipe::TreeModel_Recipe::row_draggable_vfunc(const Gtk::TreeModel::Path& path) const {
	const_iterator iterator = const_cast<TreeModel_Recipe*>(this)->get_iter(path);
	
	if(iterator){
		Row row = *iterator;
		bool is_draggable = row[columns.m_draggable];
		return is_draggable;
		
	}
	return Gtk::TreeStore::row_draggable(path);
}
bool TreeView_Recipe::TreeModel_Recipe::row_drop_possible_vfunc(const Gtk::TreeModel::Path& dest, const Gtk::SelectionData& selection_data) const{
	//dest is the path that the row would have after it has been dropped
	Gtk::TreeModel::Path dest_parent = dest;
	bool is_not_top_level = dest_parent.up();
	
	if (!is_not_top_level || dest_parent.empty()){ //toplevel
		return true;
	}else{
		return false;
	}
}

void TreeView_Recipe::on_listView_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selectionData, guint info, guint time){
	std::cout << "on_listView_drag_data_get" << std::endl;
	selectionData.set(selectionData.get_target(), 8, (const guchar*) "I'm Data!", 9);

}
void TreeView_Recipe::on_listView_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time){
	std::cout << "on_listView_drag_data_received" << std::endl;
	const int length = selection_data.get_length();
	
	if((length >= 0) && (selection_data.get_format() == 8)){
		std::cout << "received: " << selection_data.get_data_as_string() << std::endl;
	}
}

void TreeView_Recipe::addTaskToView(Task::Task_ptr t){
	Gtk::TreeModel::Row recipeRow = *(treeModel->append());
	recipeRow[treeModel->columns.m_col_id] = t->getID();
	recipeRow[treeModel->columns.m_col_name] = t->getName();
	recipeRow[treeModel->columns.m_col_type] = t->getType();
	recipeRow[treeModel->columns.m_draggable] = true;
	
	
	//add child recipes to view if task is a recipe
	Recipe::Recipe_ptr r = std::dynamic_pointer_cast<Recipe>(t);
	if (r != nullptr){
		addChildRecipesToView(r, &recipeRow);
	} 
}

void TreeView_Recipe::addChildRecipesToView(Recipe::Recipe_ptr recipe, Gtk::TreeModel::Row* motherRow){
	std::vector<Task::Task_ptr> tasks = recipe->getTasks();
	
	for(std::vector<Task::Task_ptr>::iterator it = tasks.begin(); it != tasks.end(); it++){
		Task::Task_ptr t = *it; //current Task
		
		Gtk::TreeModel::Row childrow = *(treeModel->append(motherRow->children()));
		childrow[treeModel->columns.m_col_id] = t->getID();
		childrow[treeModel->columns.m_col_name] = t->getName();
		childrow[treeModel->columns.m_draggable] = false;
		childrow[treeModel->columns.m_col_type] = t->getType();
		
		Recipe::Recipe_ptr childRecipe = std::dynamic_pointer_cast<Recipe>(t);
		if (childRecipe != nullptr){ // task is recipe
			addChildRecipesToView(childRecipe, &childrow);
		}
	}
}

///@todo mutex for recipes ?
void TreeView_Recipe::setTasks(std::vector<Task::Task_ptr> r){
	treeModel->clear(); //remove all rows
	
	TreeView_Recipe::tasks.clear(); //clear internal receipes variable
	
	//add all recipes
	for (std::vector<Task::Task_ptr>::iterator it = r.begin(); it != r.end(); it++){
		addTask(*it);
	}
}

void TreeView_Recipe::addTask(Task::Task_ptr task){
	addTaskToView(task);
	
	//just add task to tasks list if it is not already in the list
	if (tasks.empty()){ //tasks list empty -> add task
		tasks.push_back(task);
	}else{ //tasks list is not empty -> check if task is already in list
		if (tasks.at(tasks.size()-1)->getID() < (task)->getID()){ // the ID of task is greater than the last id  -> add at the end
			tasks.push_back(task); //add at the end
		}else{ // the id of task is not the greates in recipes -> maybe add it in the middle of the list
			for(std::vector<Task::Task_ptr>::iterator it = tasks.begin(); it != tasks.end(); it++){
				if ((*it)->getID() == task->getID()){ //found in list
					break; //jump out of for loop and don't insert recipe
				}else if((*it)->getID() > task->getID()){ //insert at it
					tasks.insert(it, task);
					break; //jump out of for loop
				}
			}
		}
	}
}

bool TreeView_Recipe::select_function(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool){
	Gtk::TreeModel::Path activated_path = path;
	activated_path.up();
	
	if (!activated_path.up()){ //toplayer
		return true;
	}
	return false;
}

Task::Task_ptr TreeView_Recipe::getSelectedTask() const{
	Task::Task_ptr task;
	TreeModel_Recipe::iterator iter = treeView_recipe_selection->get_selected();
	if(iter) { //something is selected
		TreeModel_Recipe::Row row = *iter;
		int task_id = row[treeModel->columns.m_col_id];
		
		//search recipe in list
		for (std::vector<Task::Task_ptr>::const_iterator cit = tasks.cbegin(); cit != tasks.cend(); cit++){
			if ((*cit)->getID() == task_id){ // found the recipe
				task = *cit;
				break;
			}
		}
//		std::cout << "TreeView_Recipe::getSelectedTask - not found -  task_id: " << task_id << std::endl;
	}else{ //nothing is selected
		std::cout << "TreeView_Recipe::getSelectedTask - nothing selected" << std::endl;
	}
	
	return task;
}

std::vector<Task::Task_ptr> TreeView_Recipe::getTasks() const{
	std::vector<Task::Task_ptr> retTasks;
	
	for(Gtk::TreeModel::Children::iterator it = treeModel->children().begin(); it != treeModel->children().end(); ++it){ //for each row
		Gtk::TreeModel::Row row = *it;
		
		//search recipe
		for(std::vector<Task::Task_ptr>::const_iterator it2 = tasks.cbegin(); it2 != tasks.cend(); it2++){
			if ((*it2)->getID() == row[treeModel->columns.m_col_id]){ //found recipe
				retTasks.push_back(*it2);
				break; //jump out of for loop
			}
		}
	} 
	return retTasks;
}

void TreeView_Recipe::deleteSelectedRow(){
	TreeModel_Recipe::iterator iter = treeView_recipe_selection->get_selected();
	if(iter) { //anything is selected
		TreeModel_Recipe::Row row = *iter;
		int task_id = row[treeModel->columns.m_col_id];
		bool anotherRefInGui = false;
		
		treeModel->erase(iter); // remove row from gui
		//search if task has another reference in gui
		for(Gtk::TreeModel::Children::iterator it = treeModel->children().begin(); it != treeModel->children().end(); ++it){ //for each row
			Gtk::TreeModel::Row row = *it;
			if (row[treeModel->columns.m_col_id] == task_id){ //found another reference
				anotherRefInGui = true;
				break;
			}
		}
		std::cout << "anotherRefInGui: " << anotherRefInGui <<std::endl;
		if (!anotherRefInGui){ //no other reference -> remove recipe from list
			//search recipe in list
			for (std::vector<Task::Task_ptr>::iterator it = tasks.begin(); it != tasks.end(); it++){
				if ((*it)->getID() == task_id){ // found the recipe
					tasks.erase(it);
					std::cout << "removed task from list" << std::endl;
					break;
				}
			}
		}
	}
}

void TreeView_Recipe::set_TreeView_sensitive(bool value){
	treeView_recipe->set_sensitive(value);
}

Recipe::Recipe_ptr TreeView_Recipe::getAllTasksAsOneRecipe(std::string name, std::string comment) const{
	Recipe::Recipe_ptr r = Recipe::create(name);
	std::vector<Task::Task_ptr> tasks = getTasks();
	
	r->setComment(comment);
	
	for (std::vector<Task::Task_ptr>::const_iterator cit = tasks.cbegin(); cit != tasks.cend(); cit++){
		r->addTask(*cit);
	}
	
	return r;
}

void TreeView_Recipe::clear(){
	treeModel->clear();
	tasks.clear();
}