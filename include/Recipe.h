#pragma once
/**
 * @file Recipe.h
 * 
 * @class Recipe
 * @author Nils Bosbach
 * @date 14.04.2019
 * @brief stores multiple Tasks -> group a bunch of taks to a recipe
 */
#include "Spectrometer.h"
#include "Task.h"
#include "Uc_Connection.h"

#include <vector>
#include <string>
#include <mutex>
#include <pthread.h>
#include <memory>

class Recipe : public Task{
public:
	typedef std::shared_ptr<Recipe> Recipe_ptr;
	
private:
	
	/**
	 * @brief set the name of the Recipe
	 * @param name 
	 */
	Recipe(std::string name = "");
	
	/**
	 * @brief load a Recipe from file (.xml or .csv)
	 * @param path the path of the file the recipe should be loaded from
	 * @param ucControl a Uc_Control Object which handles the control to activate the Pads
	 * 
	 * if basic Recipe is true the file should have the following structure:<br>
	 * <time>;<pad1>;<pad2>;...;<padn> <br>
	 * <time>;<pad1>;<pad2>;...;<padn> <br>
	 * ...
	 */
	
public:
	
	static Recipe_ptr create(const std::string& name = "");
	
	
	/**
	 * @brief copy constructor
	 * @param r the Recipe which should be copied
	 * 
	 */
//	Recipe(const Recipe& r);
	virtual ~Recipe();
	
	/**
	 * @brief add a Task to the Recipe
	 * @param t the Task which should be added
	 */
	void addTask(Task_ptr t);
	
	/**
	 * @brief executes all Tasks
	 * @param data contains the data which is captured during the execution of the recipe
	 * @param executeNext if false the Recipe will stop after the current task
	 */
	void execute(ExperimentData::ExperimentData_ptr data, bool* executeNext) override;
	
	/**
	 * @brief get all Tasks of the recipe
	 * @return vector of all Tasks of the recipe
	 */
	const std::vector<Task::Task_ptr> getTasks() const;
	
	/**
	 * @brief returns the type as std::string
	 * @return "recipe"
	 */
	virtual std::string getType() const;
	
	/**
	 * @brief check whether the recipe consits of other recipes
	 * @return true if no task in tasks is a recipe
	 */
	bool isBasicRecipe();
	
	/**
	 * @brief saves the recipe as xml file
	 * @param file destination where the Recipe should be saved
	 */
	void save(std::string file);
	
	/**
	 * @brief saves the recipe as xml file
	 */
	void save();
	
	/**
	 * @brief compares this Recipe whith another
	 * @param rhs the Recipe this Recipe should be compared with
	 * @return true if the ID of this recipe matches the ID of the other Recipe
	 */
	bool operator==(const Recipe& rhs) const { return this->getID() == rhs.getID();}
	
	/**
	 * @brief saves the task as part of a xml file
	 * @param doc the xml document which should contain the task
	 * @param externElements if true, links to other files are allowed
	 * @return the part of the xml file
	 */
	virtual tinyxml2::XMLElement* toXMLElement(tinyxml2::XMLDocument *doc, bool externElements = false) override;
	
	void set_defaultFolderRecipes(std::string path);
	
	/**
	 * @brief load a recipe from a .csv or .xml file
	 * @param path the path of the file
	 * @param s pointer to a spectrometer
	 * @param name name of the recipe
	 * @param comment comment of the recipe
	 * @return pointer to the created object.
	 */
	static Recipe_ptr loadRecipe(std::string path, Spectrometer* s, std::string name = "", std::string comment = "");
	
	/**
	 * @brief create a Recipe from data defined in a xml file and return a smart pointer to the created object
	 * @param task_element part of the xml file which contains the elements of the recipe
	 * @return smart pointer to the created object
	 */
	static Recipe_ptr loadRecipe(tinyxml2::XMLElement* task_element, Spectrometer *s);
	
	/** 
	 * @brief check, if a recipe with a specific name exists at the moment
	 * @param name name of the recipe
	 * @return true, if a recipe with the name exists at the moment
	 */
	static bool recipeWithNameExists(std::string name);
	
	/**
	 * @brief get a smart pointer of a recipe by the name of the recipe
	 * @param name name of the recipe
	 * @return smart pointer to the recipe with the name
	 */
	static Recipe_ptr getRecipeByName(std::string name);
	
	/**
	 * @brief set the name of the recipe
	 * @param name name of the recipe
	 */
	virtual void setName(std::string name) override;
	
	/**
	 * @brief get the path where the recipe has been saved / has been loaded from
	 * @return path of the recipe
	 */
	std::string getPath() const;
	
	/**
	 * @brief set the path of the recipe
	 * @param path path of the recipe
	 */
	void setPath(std::string path);
	
	/**
	 * @brief get a list of all devices, which have to be connected to the raspberry pi to run the task
	 * @return list of devices, which are needed to run the task
	 */
	virtual std::list<Task::DEVICES> getNecessaryDevices() override;
	
private:
	
	/// all steps of the Recipe
	std::vector<Task_ptr> tasks;
	
	/// used to synchronize tasks
	std::mutex tasksMtx;
	
	/// path to the file the Recipe was loaded from / should be saved to @todo implement save recipe
	std::string file_path;
	
	bool changed;
	static std::vector<std::weak_ptr<Recipe>> all_recipes;
	static std::mutex all_recipesMtx;
};
