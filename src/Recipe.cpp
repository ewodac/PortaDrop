#include "Recipe.h"
#include "PadTask.h"
#include "SpectrometerTask.h"
#include "FSHelper.h"
#include "ImpAnalyserTask.h"
#include "TransImpTask.h"
#include "DelayTask.h"
#include "I2CFreqTask.h"
#include "I2CVoltageTask.h"

#include <iostream>
#include <fstream>
#include <tinyxml2.h>
#include <stdexcept>

std::vector<std::weak_ptr<Recipe>> Recipe::all_recipes;
std::mutex Recipe::all_recipesMtx;

Recipe::Recipe(std::string name): Task(){
	file_path = "";
	changed = true;
	setName(name);
}

Recipe::Recipe_ptr Recipe::create(const std::string& name){
	Recipe_ptr r = Recipe_ptr(new Recipe(name));
	
	//add a weak_ptr to the global list
	std::weak_ptr<Recipe> weak_r = r;
	all_recipesMtx.lock();
	all_recipes.push_back(weak_r);
	all_recipesMtx.unlock();
	
	return r;
}


Recipe::~Recipe(){
	
	//remove recipe from global vector
	all_recipesMtx.lock();
	for(std::vector<std::weak_ptr<Recipe>>::iterator it = all_recipes.begin(); it != all_recipes.end(); it++){
		if (Recipe_ptr r = it->lock()){ // pointer is not expired
			if (r->getID() == id){
				std::cout << "removed recipe from global list" << std::endl;
				all_recipes.erase(it);
				break;
			}
		}else{
			std::cout << "expired pointer in list" << std::endl;
			all_recipes.erase(it);
			break;
		}
	}
	all_recipesMtx.unlock();
}

void Recipe::addTask(Task_ptr t){
	tasksMtx.lock();
	tasks.push_back(t);
	changed = true;
	tasksMtx.unlock();
}

void Recipe::execute(ExperimentData::ExperimentData_ptr data, bool* executeNext){
	
	// recipe should not be executed
	if (!(*executeNext)){
		return;
	}
	
	tasksMtx.lock();
	try{
		addLogEvent(Log_Event::create("start recipe", "recipe " + name + "(" + std::to_string(id) + ")", Log_Event::TYPE::LOG_INFO));
		for(std::vector<Task_ptr>::iterator it = tasks.begin(); it != tasks.end(); it++){
			Task_ptr t = *it;
			if (*executeNext){
				t->execute(data, executeNext);
			}else{
				break;
			}
		}
		addLogEvent(Log_Event::create("end recipe", "recipe " + name + "(" + std::to_string(id) + ")", Log_Event::TYPE::LOG_INFO));
	}catch(std::runtime_error r){
		tasksMtx.unlock();
		throw;
	}catch(std::invalid_argument r){
		tasksMtx.unlock();
		throw;
	}
	
	tasksMtx.unlock();
}

const std::vector<Task::Task_ptr> Recipe::getTasks() const{
	return tasks;
}
std::string Recipe::getType() const{
	return "recipe";
}

bool Recipe::isBasicRecipe(){
	tasksMtx.lock();
	bool retval = true;
	for(std::vector<Task_ptr>::const_iterator it = tasks.cbegin(); it != tasks.cend(); it++){
		if (std::dynamic_pointer_cast<Recipe>(*it) != nullptr){ // found a recipe
			retval = false;
			break;
		}
	}
	tasksMtx.unlock();
	return retval;
}

void Recipe::save(std::string file){
	file_path = file;
	changed = true;
	save();
}
void Recipe::save(){
	if (changed){
		if (!file_path.empty()){
			if (FSHelper::endsWith(file_path, ".csv")){
				
			}else{ // save xml file
				tasksMtx.lock();
				
				tinyxml2::XMLDocument doc;
				doc.InsertEndChild(toXMLElement(&doc, false));
				doc.SaveFile(file_path.c_str());
				std::cout << "saved recipe to " << file_path << std::endl;
				
				tasksMtx.unlock();
			}
			changed = false;
		}
	}
}

tinyxml2::XMLElement* Recipe::toXMLElement(tinyxml2::XMLDocument *doc, bool externElements){
	tinyxml2::XMLElement* xmlTaskElement = doc->NewElement("Recipe");
	
	xmlTaskElement->SetAttribute("name", name.c_str());
	xmlTaskElement->SetAttribute("comment", comment.c_str());
	
	if (externElements){ // save recipe in own file and link the path
		
		xmlTaskElement->SetAttribute("path", file_path.c_str());
		std::cout << "file_path: " << file_path << std::endl;
		
		save();
	}else{
		for(std::vector<Task_ptr>::const_iterator cit = Recipe::tasks.cbegin(); cit != tasks.cend(); cit++){
			xmlTaskElement->InsertEndChild((*cit)->toXMLElement(doc, externElements));
		}
	}
	
	return xmlTaskElement;
}

Recipe::Recipe_ptr Recipe::loadRecipe(tinyxml2::XMLElement* task_element, Spectrometer *s){
	std::string path = "", name = "", comment = "";
	
	//get name
	if (task_element->FindAttribute("name") != nullptr){
			name = task_element->FindAttribute("name")->Value();
	}
	
	//get comment
	if (task_element->FindAttribute("comment") != nullptr){
		comment = task_element->FindAttribute("comment")->Value();
	}
	
	Recipe_ptr r = getRecipeByName(name);
	if (r != nullptr) return r;
	
	//recipe has not been loaded yet
	
	if (task_element->FindAttribute("path") != nullptr){ //path has been set -> load from file
		path = task_element->FindAttribute("path")->Value();
		
		return loadRecipe(path, s, name, comment);
	}else{ //direct load
		r = create(name);
		r->setComment(comment);
		
		tinyxml2::XMLNode* task = task_element->FirstChild();
		while (task!= nullptr){ // add all tasks
			tinyxml2::XMLElement* currentTask = dynamic_cast<tinyxml2::XMLElement*>(task);
			
			if (currentTask != 0){
				if (std::string(currentTask->Name()).compare("Recipe") == 0){
					r->addTask(loadRecipe(currentTask, s));
				}else if(std::string(currentTask->Name()).compare("PadTask") == 0){
					r->addTask(PadTask::loadPadTask(currentTask));
				}else if(std::string(currentTask->Name()).compare("SpectrometerTask") == 0){
					r->addTask(SpectrometerTask::loadSpectrometerTask(currentTask, s));
				}else if(std::string(currentTask->Name()).compare("ImpAnalyserTask") == 0){
					r->addTask(ImpAnalyserTask::loadImpAnalyserTask(currentTask));
				}else if(std::string(currentTask->Name()).compare("TransImpTask") == 0){
					r->addTask(TransImpTask::loadTransImpTask(currentTask));
				}else if(std::string(currentTask->Name()).compare("DelayTask") == 0){
					r->addTask(DelayTask::loadDelayTask(currentTask));
				}else if(std::string(currentTask->Name()).compare("FreqTask") == 0){
					r->addTask(I2CFreqTask::loadFreqTask(currentTask));
				}else if(std::string(currentTask->Name()).compare("VoltageTask") == 0){
					r->addTask(I2CVoltageTask::loadVoltageTask(currentTask));
				}
			}
			task = task->NextSibling();
		}
		return r;
	}
	return r; // never reached
}
Recipe::Recipe_ptr Recipe::loadRecipe(std::string path, Spectrometer *s, std::string name, std::string comment){
	
	Recipe_ptr r = getRecipeByName(name);
	if (r != nullptr) return r;
	
	//recipe has not been loaded -> load recipe
	
	if(FSHelper::endsWith(path, ".csv")){
		
		r = create(path.substr(path.find_last_of("/") + 1));
		r->setPath(path);
		
		std::ifstream file(path);
		if (file.is_open()){
			std::string line;
			while(std::getline(file, line)){
				if (!line.empty()){
					r->addTask(PadTask::getPadTaskFromString(line));
				}
			}
			return r;
		}else{
			std::cout << "error opening file " << path << std::endl;
		}
	} else if (FSHelper::endsWith(path, ".xml")){
		tinyxml2::XMLDocument doc;
		
		if (doc.LoadFile(path.c_str()) == 0){ // no error
			tinyxml2::XMLElement* recipe = dynamic_cast<tinyxml2::XMLElement*>(doc.FirstChildElement("Recipe"));
			if (recipe != nullptr){
				r =  loadRecipe(recipe, s);
				r->setPath(path);
				
				return r;
			}else{ //error, no recipe xml file
				
			}
		} else {
//			addLogEvent(Log_Event::create("error opening xml file", "opening xml file '" + path + "' failed"));
		}
	} else { // unsupported file ending
//		addLogEvent(Log_Event::create("unsupported file", "Recipe::Recipe(std::string path, Uc_Control *ucControl) - the file " + path + "is not supported", Log_Event::TYPE::LOG_ERROR));
	}
	return r;
}
void Recipe::setName(std::string name){
	if (recipeWithNameExists(name)){
		std::cout << "warning - recipe with name '" << name << "' already exists" << std::endl;
	}
	Task::setName(name);
}

std::string Recipe::getPath() const{
	return file_path;
}
void Recipe::setPath(std::string path){
	file_path = path;
}
bool Recipe::recipeWithNameExists(std::string name){
	return (getRecipeByName(name) != nullptr);
}
Recipe::Recipe_ptr Recipe::getRecipeByName(std::string name){
	Recipe_ptr retVal;
	
	all_recipesMtx.lock();
	for (std::vector<std::weak_ptr<Recipe>>::iterator it = all_recipes.begin(); it != all_recipes.end(); it++){
		if (Recipe_ptr r = it->lock()){ // pointer is not expired
			if (r->getName().compare(name) == 0){ //found recipe
				retVal = r;
				break;
			}
		}else{
			std::cout << "found expired weak_ptr in global list" << std::endl;
		}
	}
	all_recipesMtx.unlock();
	return retVal;
}

std::list<Task::DEVICES> Recipe::getNecessaryDevices(){
	std::list<Task::DEVICES> devices;
	
	// for all tasks of the recipe
	for (std::vector<Task_ptr>::iterator it_tasks = tasks.begin(); it_tasks != tasks.end(); it_tasks++){
		Task::Task_ptr t = *it_tasks;
		std::list<Task::DEVICES> task_devices = t->getNecessaryDevices();
		
		// for all necessary devices of the current task
		for(std::list<Task::DEVICES>::iterator it_task_devices = task_devices.begin(); it_task_devices != task_devices.end(); it_task_devices++){
			Task::DEVICES device = *it_task_devices; // current devices
			bool device_already_in_list = false; // is the current device already in the list?
			
			//search, if the current device is already in the return list
			for(std::list<Task::DEVICES>::iterator it_devices = devices.begin(); it_devices != devices.end(); it_devices++){
				if(*it_devices == device){
					device_already_in_list = true;
					break;
				}
			}
			
			if(!device_already_in_list){
				devices.push_back(device);
			}
		}
	}
	
	return devices;
}
