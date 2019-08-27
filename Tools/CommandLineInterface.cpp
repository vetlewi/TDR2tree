#include "CommandLineInterface.h"

using namespace std;

CommandLineInterface::CommandLineInterface()
{
  fMaximumFlagLength = 0;
  fFlags.clear();
  fValues.clear();
  fMaximumTypeLength = 0;
  fTypes.clear();
  fMaximumCommentLength = 0;
  fComments.clear();
}

bool CommandLineInterface::CheckFlags(int argc, char* argv[], const bool& Debug)
{
  size_t found=0;
  if(argc == 1)
    {
      for(size_t i = 0; i < fFlags.size(); i++)
	{
	  if(fTypes[i].empty())
	    cout<<fComments[i]<<endl<<endl;
	}
      cout<<"use "<<argv[0]<<" with following flags:"<<endl;
      for(size_t i = 0; i < fFlags.size(); i++)
	{
	  if(fTypes[i] == "bool")
	    cout<<"        ["<<setw(fMaximumFlagLength+fMaximumTypeLength)<<left<<fFlags[i]<<"   : "<<fComments[i]<<"]"<<endl;
	  else if(!fTypes[i].empty())
	    cout<<"        ["<<setw(fMaximumFlagLength)<<left<<fFlags[i]<<" <"<<setw(fMaximumTypeLength)<<left<<fTypes[i]<<">: "<<fComments[i]<<"]"<<endl;
	}

      return true;
    }

  for(int i = 1; i < argc; i++)
    {
      for(size_t j = 0; j < fFlags.size(); j++)
	{
      found = j;
	  if(argv[i] == fFlags[j])
	    {
	      //bool doesn't need any value to be read
	      if(fTypes[j] == "bool")
		{
          *(static_cast<bool*>(fValues[j])) = true;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      //if not bool check whether there are more arguments (with values) coming
	      else if(i+1 >= argc)
		{
		  cerr<<"Error in CheckFlags, flag "<<fFlags[j]<<" needs additional arguments"<<endl;
		  return false;
		}
	      else if(fTypes[j] == "char*")
		{
          *(static_cast<char**>(fValues[j])) = argv[i+1];
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "string")
		{
          *(static_cast<string*>(fValues[j])) = argv[i+1];
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "int")
		{
          *(static_cast<int*>(fValues[j])) = atoi(argv[i+1]);
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "size_t")
		{
          *(static_cast<size_t*>(fValues[j])) = size_t(atoll(argv[i+1]));
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "long long")
		{
          *(static_cast<long long*>(fValues[j])) = atoll(argv[i+1]);
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "double")
		{
          *(static_cast<double*>(fValues[j])) = atof(argv[i+1])*fFactors[j];
		  i++;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "vector<char*>")
		{
		  i++;
		  //as long as there are arguments left and no new flag is found (flags start with -) => read another value
		  while(i < argc)
		    {
		      if(argv[i][0] != '-')
			{
              (*(static_cast<vector<char*>*>(fValues[j]))).push_back(argv[i]);
			  i++;
			}
		      else
			{
			  break;
			}
		    }

		  i--;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "vector<string>")
		{
		  i++;
		  //as long as there are arguments left and no new flag is found (flags start with -) => read another value
		  while(i < argc)
		    {
		      if(argv[i][0] != '-')
			{
              (*(static_cast<vector<string>*>(fValues[j]))).push_back(argv[i]);
			  i++;
			}
		      else
			{
			  break;
			}
		    }

		  i--;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "vector<int>")
		{
		  i++;
		  //as long as there are arguments left and no new flag is found (flags start with -) => read another value
		  while(i < argc)
		    {
		      if(argv[i][0] != '-')
			{
              (*(static_cast<vector<int>*>(fValues[j]))).push_back(atoi(argv[i]));
			  i++;
			}
		      else
			{
			  break;
			}
		    }

		  i--;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "vector<long long>")
		{
		  i++;
		  //as long as there are arguments left and no new flag is found (flags start with -) => read another value
		  while(i < argc)
		    {
		      if(argv[i][0] != '-')
			{
              (*(static_cast<vector<long long>*>(fValues[j]))).push_back(atoll(argv[i]));
			  i++;
			}
		      else
			{
			  break;
			}
		    }

		  i--;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	      else if(fTypes[j] == "vector<double>")
		{
		  i++;
		  //as long as there are arguments left and no new flag is found (flags start with -) => read another value
		  while(i < argc)
		    {
		      if(argv[i][0] != '-')
			{
              (*(static_cast<vector<double>*>(fValues[j]))).push_back(atof(argv[i])*fFactors[j]);
			  i++;
			}
		      else
			{
			  break;
			}
		    }

		  i--;
		  break;//found the right flag for this argument so the flag loop can be stopped
		}
	    }//if(argv[i] == flags[j])
	}//for(j = 0; j < flags.size(); j++)

      if(found == fFlags.size())//this means no matching flag was found
	{
	  cerr<<"flag "<<argv[i]<<" unknown"<<endl;
	}
      else if(Debug)
	{
	  cout<<"found flag "<<i<<" = "<<argv[i]<<endl;
	}
    }//for(i = 1; i < argc; i++)

  if(Debug)
    {
      cout<<*this<<endl;
    }

  return true;
}

ostream& operator <<(ostream &os,const CommandLineInterface &obj)
{
  os<<"command line flags are:"<<endl;
  for(size_t i = 0; i < obj.fValues.size(); i++)
    {
      if(obj.fTypes[i] == "bool")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<bool*>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "char*")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<char**>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "string")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<string*>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "int")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<int*>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "long long")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<long*>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "double")
	{
      cout<<obj.fFlags[i]<<": "<<*(static_cast<double*>(obj.fValues[i]))<<endl;
	}
      else if(obj.fTypes[i] == "vector<char*>")
	{
	  cout<<obj.fFlags[i]<<": ";
      for (size_t j = 0 ; j < static_cast<vector<char*>*>(obj.fValues[i])->size() ; ++j)
        {
          cout<< (*static_cast<vector<char*>*>(obj.fValues[i]))[j] << " ";
        }
	  cout<<endl;
	}
      else if(obj.fTypes[i] == "vector<string>")
	{
	  cout<<obj.fFlags[i]<<": ";
      for (size_t j = 0 ; j < static_cast<vector<string>*>(obj.fValues[i])->size() ; ++j)
        {
          cout<< (*static_cast<vector<string>*>(obj.fValues[i]))[j] << " ";
        }
	  cout<<endl;
	}
      else if(obj.fTypes[i] == "vector<int>")
	{
	  cout<<obj.fFlags[i]<<": ";
      for (size_t j = 0 ; j < static_cast<vector<int>*>(obj.fValues[i])->size() ; ++j)
        {
          cout<< (*static_cast<vector<int>*>(obj.fValues[i]))[j] << " ";
        }
	  cout<<endl;
	}
      else if(obj.fTypes[i] == "vector<long long>")
	{
	  cout<<obj.fFlags[i]<<": ";
      for (size_t j = 0 ; j < static_cast<vector<long long>*>(obj.fValues[i])->size() ; ++j)
        {
          cout<< (*static_cast<vector<long long>*>(obj.fValues[i]))[j] << " ";
        }
	  cout<<endl;
	}
      else if(obj.fTypes[i] == "vector<double>")
	{
	  cout<<obj.fFlags[i]<<": ";
      for (size_t j = 0 ; j < static_cast<vector<double>*>(obj.fValues[i])->size() ; ++j)
	    {
          cout<< (*static_cast<vector<double>*>(obj.fValues[i]))[j] << " ";
	    }
	  cout<<endl;
	}
    }

  return os;
}

void CommandLineInterface::Add(const char* comment)
{
  fFlags.push_back(string());
  fValues.push_back(static_cast<void*>(nullptr));
  fTypes.push_back(string());
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, bool* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("bool") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("bool");
  fTypes.push_back(string("bool"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, char** value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("char*") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("char*");
  fTypes.push_back(string("char*"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, string* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("string") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("string");
  fTypes.push_back(string("string"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, int* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("int") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("int");
  fTypes.push_back(string("int"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, size_t* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("int") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("size_t");
  fTypes.push_back(string("size_t"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, long long* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("long long") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("long long");
  fTypes.push_back(string("long long"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, double* value, double factor)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("double") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("double");
  fTypes.push_back(string("double"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(factor);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<char*>* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("vector<char*>") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("vector<char*>");
  fTypes.push_back(string("vector<char*>"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<string>* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("vector<string>") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("vector<string>");
  fTypes.push_back(string("vector<string>"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<int>* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("vector<int>") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("vector<int>");
  fTypes.push_back(string("vector<int>"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<long long>* value)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("vector<long long>") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("vector<long long>");
  fTypes.push_back(string("vector<long long>"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(1.);
}

void CommandLineInterface::Add(const char* flag, const char* comment, vector<double>* value, double factor)
{
  if(strlen(flag) > fMaximumFlagLength)
    fMaximumFlagLength = strlen(flag);
  fFlags.push_back(string(flag));
  fValues.push_back(static_cast<void*>(value));
  if(strlen("vector<double>") > fMaximumTypeLength)
    fMaximumTypeLength = strlen("vector<double>");
  fTypes.push_back(string("vector<double>"));
  if(strlen(comment) > fMaximumCommentLength)
    fMaximumCommentLength = strlen(comment);
  fComments.push_back(string(comment));
  fFactors.push_back(factor);
}
