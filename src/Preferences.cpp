#include "Preferences.h"

#include <tinyxml2.h>
#include <iostream>

Preferences::Preferences(){

}

Preferences::~Preferences(){
	
}

void Preferences::saveToFile(std::string path){
	tinyxml2::XMLDocument doc;
	
	/* <------------ header ------------> */
	tinyxml2::XMLElement* mainElement = doc.NewElement("preferences");
	doc.InsertEndChild(mainElement);
	
	/* <------------ recipes paths ------------> */
	tinyxml2::XMLNode* pref_recipesPaths = mainElement->InsertEndChild(doc.NewElement("recipes_folders"));
	pref_recipesPaths->InsertEndChild(doc.NewComment( "all paths to folders which contain recipes" ) );
	
	folders_recipesMtx.lock();
	for(std::vector<std::string>::const_iterator cit = folders_recipes.cbegin(); cit != folders_recipes.cend(); cit++){
		tinyxml2::XMLElement* pref_recipesPaths_path = doc.NewElement("path");
		pref_recipesPaths_path->SetAttribute("destination", (*cit).c_str());
		pref_recipesPaths->InsertEndChild(pref_recipesPaths_path);
	}
	folders_recipesMtx.unlock();
	
	/* <------------ folder myRecipes ------------> */
	tinyxml2::XMLElement* pref_folderMyRecipes = doc.NewElement("folder_myRecipes");
	pref_folderMyRecipes->SetAttribute("path", folder_myRecipes.c_str());
	mainElement->InsertEndChild(pref_folderMyRecipes);
	
	/* <------------ folder projects ------------> */
	tinyxml2::XMLElement* pref_folderProjects = doc.NewElement("folder_projects");
	pref_folderProjects->SetAttribute("path", folder_projects.c_str());
	mainElement->InsertEndChild(pref_folderProjects);
	
	/* <------------ folder projects ------------> */
	if (!currentProject.empty()){
		tinyxml2::XMLElement* pref_currentProject = doc.NewElement("currentProject");
		pref_folderProjects->SetAttribute("name", currentProject.c_str());
		mainElement->InsertEndChild(pref_currentProject);
	}
	
//	/* <------------ test pref ------------> */
//	tinyxml2::XMLNode* pref_test = mainElement->InsertEndChild(doc.NewElement("test_pref"));
//	pref_test->InsertEndChild(doc.NewComment( "this a test pref" ) );
//	tinyxml2::XMLElement* pref_test_child = doc.NewElement("testChild");
//	pref_test_child->SetAttribute("name", "I am a test attribute");
//	pref_test->InsertEndChild(pref_test_child);
	
	
	
	doc.SaveFile(path.c_str());
}

int Preferences::loadFromFile(std::string path){
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(path.c_str()) == 0){ // no error
		
		//folders_recipes
		folders_recipesMtx.lock();
		folders_recipes.clear();
		//add all folder
		tinyxml2::XMLElement* path = doc.FirstChildElement("preferences")->FirstChildElement("recipes_folders")->FirstChildElement();
		while (path != 0){
			folders_recipes.push_back(path->FindAttribute("destination")->Value());
			path = path->NextSiblingElement("path");
		}
		folders_recipesMtx.unlock();
		
		/* <------------ folder myRecipes ------------> */
		tinyxml2::XMLElement* folder_myRecipes_path = doc.FirstChildElement("preferences")->FirstChildElement("folder_myRecipes");
		if (folder_myRecipes_path != 0){
			folder_myRecipes = folder_myRecipes_path->FindAttribute("path")->Value();
			folders_recipesMtx.lock();
			folders_recipes.push_back(folder_myRecipes);
			folders_recipesMtx.unlock();
		}else{
			folder_myRecipes = "";
		}
		
		/* <------------ folder projects ------------> */
		tinyxml2::XMLElement* folder_projects_path = doc.FirstChildElement("preferences")->FirstChildElement("folder_projects");
		if (folder_projects_path != 0){
			folder_projects = folder_projects_path->FindAttribute("path")->Value();
		}else{
			folder_projects = "";
		}
		
		/* <------------ current project ------------> */
		tinyxml2::XMLElement* name_currentProject = doc.FirstChildElement("preferences")->FirstChildElement("currentProject");
		if (name_currentProject != 0){
			if (name_currentProject->FindAttribute("name") != 0){
				currentProject = name_currentProject->FindAttribute("name")->Value();
			}
		}else{
			currentProject = "";
		}
		
		return 0;
	}
	return 1;
}

void Preferences::addRecipesFolder(std::string path){
	folders_recipesMtx.lock();
	folders_recipes.push_back(path);
	folders_recipesMtx.unlock();
}
std::vector<std::string>* Preferences::getFolders_recipes(){
	
	folders_recipesMtx.lock();
	std::vector<std::string>* retArray = new std::vector<std::string>(folders_recipes);
	folders_recipesMtx.unlock();
	return retArray;
}

void Preferences::setFolderMyRecipes(std::string path){
	folder_myRecipes = path;
}

std::string Preferences::getFolderMyRecipes() const{
	return folder_myRecipes;
}

void Preferences::setFolderProjects(std::string path){
	folder_projects = path;
	folders_recipesMtx.lock();
	folders_recipes.push_back(path);
	folders_recipesMtx.unlock();
}

std::string Preferences::getFolderProjects() const{
	return folder_projects;
}

void Preferences::setCurrentProjectName(std::string currentProject){
	Preferences::currentProject = currentProject;
}
std::string Preferences::getCurrentProjectName() const{
	return Preferences::currentProject;
}