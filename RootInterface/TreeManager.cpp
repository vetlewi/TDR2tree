//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#include "TreeManager.h"

TreeManager::TreeManager(RootFileManager *FileManager, const char *name, const char *title, Event::Base *template_event )
    : tree( FileManager->CreateTree(name, title) )
    , entry_obj( template_event->New() ){}