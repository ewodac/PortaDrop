#pragma once
/**
 * @file Preferences.h
 * 
 * @class Preferences
 * @author Nils Bosbach
 * @date 28.04.2019
 * @brief handels the preferences of the gui
 */
 #include <string>
 #include <vector>
 #include <mutex>
 
class Preferences{
	
public:
	
	/**
	 * @brief constructor
	 */
	Preferences();
	
	/**
	 * @brief virtual destructor
	 */
	virtual ~Preferences();
	
	/**
	 * @brief save the preferences as xml file
	 * @param path path where the file should be stored
	 */
	void saveToFile(std::string path);
	
	/**
	 * @brief load the preferences from a xml file 
	 * @param path path of the xml file
	 * @return 0 if no error occured
	 */
	int loadFromFile(std::string path);
	
	/**
	 * @brief add the path of a folder which contains recipes
	 * @param path path of the folder
	 */
	void addRecipesFolder(std::string path);
	
	/**
	 * @brief get a list of the folder that onatain recipes
	 * @return vector of the folders that contain recipes
	 */
	std::vector<std::string>* getFolders_recipes();
	
	/**
	 * @brief set the path where new recipes will be stored
	 * @param path path of the folder where user-created recipes are stored
	 */
	void setFolderMyRecipes(std::string path);
	
	/**
	 * @brief get the path of the folder where user-created recipes are stored
	 * @return path of the folder where user-created recipes are stored
	 */
	std::string getFolderMyRecipes() const; 
	
	/**
	 * @brief set the folder where the project data will be saved
	 * @param path folder where the project data will be saved
	 */
	void setFolderProjects(std::string path);
	
	/**
	 * @brief get the path of the folder where the project data is stored
	 * @return path of the folder where the project data is stored
	 */
	std::string getFolderProjects() const; 
	
	/**
	 * @brief set the name of the current Project
	 * @param currentProject name of the current project
	 */
	void setCurrentProjectName(std::string currentProject);
	
	/**
	 * @brief get the name of the current Project
	 * @return name of the current project
	 */
	std::string getCurrentProjectName() const;
	
private:
	std::mutex folders_recipesMtx;
	std::vector<std::string> folders_recipes;
	std::string folder_myRecipes;
	std::string folder_projects;
	
	std::string currentProject;

};

