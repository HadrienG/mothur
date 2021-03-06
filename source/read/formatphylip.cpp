/*
 *  formatphylip.cpp
 *  Mothur
 *
 *  Created by westcott on 1/13/10.
 *  Copyright 2010 Schloss Lab. All rights reserved.
 *
 */

#include "formatphylip.h"
#include "progress.hpp"

/***********************************************************************/
FormatPhylipMatrix::FormatPhylipMatrix(string df) : filename(df) {
        m->openInputFile(filename, fileHandle);
}
/***********************************************************************/
//not using nameMap
int FormatPhylipMatrix::read(NameAssignment* nameMap){
	try {
        
			float distance;
			int square, nseqs;
			string name;
			ofstream out;
			
			string numTest;
			fileHandle >> numTest >> name;
			
			if (!m->isContainingOnlyDigits(numTest)) { m->mothurOut("[ERROR]: expected a number and got " + numTest + ", quitting."); m->mothurOutEndLine(); exit(1); }
			else { convert(numTest, nseqs); }
		
            if(nameMap == NULL){
                list = new ListVector(nseqs);
                list->set(0, name);
            }
            else{
                list = new ListVector(nameMap->getListVector());
                if(nameMap->count(name)==0){        m->mothurOut("Error: Sequence '" + name + "' was not found in the names file, please correct"); m->mothurOutEndLine(); }
            }			
			
			char d;
			while((d=fileHandle.get()) != EOF){
                
				if(isalnum(d)){  //you are square
					square = 1;
					fileHandle.close();  //reset file
					
					//open and get through numSeqs, code below formats rest of file
					m->openInputFile(filename, fileHandle);
					fileHandle >> nseqs; m->gobble(fileHandle);
					
					distFile = filename + ".rowFormatted";
					m->openOutputFile(distFile, out);
					break;
				}
				if(d == '\n'){
					square = 0;
					break;
				}
			}
			
			Progress* reading;
			reading = new Progress("Formatting matrix:     ", nseqs * nseqs);
			
			//lower triangle, so must go to column then formatted row file
			if(square == 0){
				int  index = 0;
				
				ofstream outTemp;
				string tempFile = filename + ".temp";
				m->openOutputFile(tempFile, outTemp);
                
				//convert to square column matrix
				for(int i=1;i<nseqs;i++){
				
					fileHandle >> name;
					
                    if(nameMap == NULL){ list->set(i, name); }
                    else { if(nameMap->count(name)==0){        m->mothurOut("Error: Sequence '" + name + "' was not found in the names file, please correct"); m->mothurOutEndLine(); }
                    }
					
					for(int j=0;j<i;j++){
					
						if (m->control_pressed) { outTemp.close(); m->mothurRemove(tempFile); fileHandle.close();  delete reading; return 0; }
											
						fileHandle >> distance;
						
						if (distance == -1) { distance = 1000000; }
						
						if(distance < cutoff){
							outTemp << i << '\t' << j << '\t' << distance << endl;
							outTemp << j << '\t' << i << '\t' << distance << endl;
						}
						index++;
						reading->update(index);
					}
				}
				outTemp.close();
				
				//format from square column to rowFormatted
				//sort file by first column so the distances for each row are together
				string outfile = m->getRootName(tempFile) + "sorted.dist.temp";
				
				//use the unix sort 
				#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
					string command = "sort -n " + tempFile + " -o " + outfile;
					system(command.c_str());
				#else //sort using windows sort
					string command = "sort " + tempFile + " /O " + outfile;
					system(command.c_str());
				#endif
				
				if (m->control_pressed) { m->mothurRemove(tempFile); m->mothurRemove(outfile);  delete reading; return 0; }

				//output to new file distance for each row and save positions in file where new row begins
				ifstream in;
				m->openInputFile(outfile, in);
				
				distFile = outfile + ".rowFormatted";
				m->openOutputFile(distFile, out);
				
				rowPos.resize(nseqs, -1);
				int currentRow;
				int first, second;
				float dist;
				map<int, float> rowMap;
				map<int, float>::iterator itRow;
				
				//get first currentRow
				in >> first;
				currentRow = first;
				
				string firstString = toString(first);
				for(int k = 0; k < firstString.length(); k++)  {   in.putback(firstString[k]);  }
				
				while(!in.eof()) {
					if (m->control_pressed) { in.close(); out.close(); m->mothurRemove(tempFile); m->mothurRemove(distFile); m->mothurRemove(outfile);  delete reading; return 0; }

					in >> first >> second >> dist; m->gobble(in);
					
					if (first != currentRow) {
						//save position in file of each new row
						rowPos[currentRow] = out.tellp();
						
						out << currentRow << '\t' << rowMap.size() << '\t';
						
						for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
							out << itRow->first << '\t' << itRow->second << '\t';
						}
						out << endl;
						
						currentRow = first;
						rowMap.clear();
						
						//save row you just read
						rowMap[second] = dist;
						
						index++;
						reading->update(index);
					}else{
						rowMap[second] = dist;
					}
				}
				
				//print last Row
				//save position in file of each new row
				rowPos[currentRow] = out.tellp();
				
				out << currentRow << '\t' << rowMap.size() << '\t';
				
				for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
					out << itRow->first << '\t' << itRow->second << '\t';
				}
				out << endl;
				
				in.close();
				out.close();
				
				m->mothurRemove(tempFile);
				m->mothurRemove(outfile);
				
				if (m->control_pressed) {  m->mothurRemove(distFile);   delete reading; return 0; }

			}
			else{ //square matrix convert directly to formatted row file
				int index = nseqs;
				map<int, float> rowMap;
				map<int, float>::iterator itRow;
				rowPos.resize(nseqs, -1);
                
				for(int i=0;i<nseqs;i++){
					fileHandle >> name;                
									
					if(nameMap == NULL){ list->set(i, name); }
                    else { if(nameMap->count(name)==0){        m->mothurOut("Error: Sequence '" + name + "' was not found in the names file, please correct"); m->mothurOutEndLine(); }
                    }
					
					for(int j=0;j<nseqs;j++){
						if (m->control_pressed) {  fileHandle.close(); out.close(); m->mothurRemove(distFile);   delete reading; return 0; }
						
						fileHandle >> distance;
					
						if (distance == -1) { distance = 1000000; }
						
						if((distance < cutoff) && (j != i)){
							rowMap[j] = distance;
						}
						index++;
						reading->update(index);
					}
					
					m->gobble(fileHandle);
			
					//save position in file of each new row
					rowPos[i] = out.tellp();

					//output row to file
					out << i << '\t' << rowMap.size() << '\t';
					for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
						out << itRow->first << '\t' << itRow->second << '\t';
					}
					out << endl;
					
					//clear map for new row's info
					rowMap.clear();
				}
			}
			reading->finish();
			delete reading;
			fileHandle.close();
			out.close();
			
			if (m->control_pressed) { m->mothurRemove(distFile);  return 0; }
			
			list->setLabel("0");
			
			return 1;
			
			
	}
	catch(exception& e) {
               m->errorOut(e, "FormatPhylipMatrix", "read");
                exit(1);
	}
}
/***********************************************************************/
//not using nameMap
int FormatPhylipMatrix::read(CountTable* nameMap){
	try {
        
        float distance;
        int square, nseqs;
        string name;
        ofstream out;
        
        string numTest;
        fileHandle >> numTest >> name;
        
        if (!m->isContainingOnlyDigits(numTest)) { m->mothurOut("[ERROR]: expected a number and got " + numTest + ", quitting."); m->mothurOutEndLine(); exit(1); }
        else { convert(numTest, nseqs); }
		
        if(nameMap == NULL){
            list = new ListVector(nseqs);
            list->set(0, name);
        }
        else{
            list = new ListVector(nameMap->getListVector());
            nameMap->get(name);
        }			
        
        char d;
        while((d=fileHandle.get()) != EOF){
            
            if(isalnum(d)){  //you are square
                square = 1;
                fileHandle.close();  //reset file
                
                //open and get through numSeqs, code below formats rest of file
                m->openInputFile(filename, fileHandle);
                fileHandle >> nseqs; m->gobble(fileHandle);
                
                distFile = filename + ".rowFormatted";
                m->openOutputFile(distFile, out);
                break;
            }
            if(d == '\n'){
                square = 0;
                break;
            }
        }
        
        Progress* reading;
        reading = new Progress("Formatting matrix:     ", nseqs * nseqs);
        
        //lower triangle, so must go to column then formatted row file
        if(square == 0){
            int  index = 0;
            
            ofstream outTemp;
            string tempFile = filename + ".temp";
            m->openOutputFile(tempFile, outTemp);
            
            //convert to square column matrix
            for(int i=1;i<nseqs;i++){
				
                fileHandle >> name;
                
                if(nameMap == NULL){ list->set(i, name); }
                else { nameMap->get(name); }
                
                
                for(int j=0;j<i;j++){
					
                    if (m->control_pressed) { outTemp.close(); m->mothurRemove(tempFile); fileHandle.close();  delete reading; return 0; }
                    
                    fileHandle >> distance;
                    
                    if (distance == -1) { distance = 1000000; }
                    
                    if(distance < cutoff){
                        outTemp << i << '\t' << j << '\t' << distance << endl;
                        outTemp << j << '\t' << i << '\t' << distance << endl;
                    }
                    index++;
                    reading->update(index);
                }
            }
            outTemp.close();
            
            //format from square column to rowFormatted
            //sort file by first column so the distances for each row are together
            string outfile = m->getRootName(tempFile) + "sorted.dist.temp";
            
            //use the unix sort 
#if defined (__APPLE__) || (__MACH__) || (linux) || (__linux) || (__linux__) || (__unix__) || (__unix)
            string command = "sort -n " + tempFile + " -o " + outfile;
            system(command.c_str());
#else //sort using windows sort
            string command = "sort " + tempFile + " /O " + outfile;
            system(command.c_str());
#endif
            
            if (m->control_pressed) { m->mothurRemove(tempFile); m->mothurRemove(outfile);  delete reading; return 0; }
            
            //output to new file distance for each row and save positions in file where new row begins
            ifstream in;
            m->openInputFile(outfile, in);
            
            distFile = outfile + ".rowFormatted";
            m->openOutputFile(distFile, out);
            
            rowPos.resize(nseqs, -1);
            int currentRow;
            int first, second;
            float dist;
            map<int, float> rowMap;
            map<int, float>::iterator itRow;
            
            //get first currentRow
            in >> first;
            currentRow = first;
            
            string firstString = toString(first);
            for(int k = 0; k < firstString.length(); k++)  {   in.putback(firstString[k]);  }
            
            while(!in.eof()) {
                if (m->control_pressed) { in.close(); out.close(); m->mothurRemove(tempFile); m->mothurRemove(distFile); m->mothurRemove(outfile);  delete reading; return 0; }
                
                in >> first >> second >> dist; m->gobble(in);
                
                if (first != currentRow) {
                    //save position in file of each new row
                    rowPos[currentRow] = out.tellp();
                    
                    out << currentRow << '\t' << rowMap.size() << '\t';
                    
                    for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
                        out << itRow->first << '\t' << itRow->second << '\t';
                    }
                    out << endl;
                    
                    currentRow = first;
                    rowMap.clear();
                    
                    //save row you just read
                    rowMap[second] = dist;
                    
                    index++;
                    reading->update(index);
                }else{
                    rowMap[second] = dist;
                }
            }
            
            //print last Row
            //save position in file of each new row
            rowPos[currentRow] = out.tellp();
            
            out << currentRow << '\t' << rowMap.size() << '\t';
            
            for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
                out << itRow->first << '\t' << itRow->second << '\t';
            }
            out << endl;
            
            in.close();
            out.close();
            
            m->mothurRemove(tempFile);
            m->mothurRemove(outfile);
            
            if (m->control_pressed) {  m->mothurRemove(distFile);   delete reading; return 0; }
            
        }
        else{ //square matrix convert directly to formatted row file
            int index = nseqs;
            map<int, float> rowMap;
            map<int, float>::iterator itRow;
            rowPos.resize(nseqs, -1);
            
            for(int i=0;i<nseqs;i++){
                fileHandle >> name;                
                
                if(nameMap == NULL){ list->set(i, name); }
                else { nameMap->get(name); }
                
                for(int j=0;j<nseqs;j++){
                    if (m->control_pressed) {  fileHandle.close(); out.close(); m->mothurRemove(distFile);   delete reading; return 0; }
                    
                    fileHandle >> distance;
					
                    if (distance == -1) { distance = 1000000; }
                    
                    if((distance < cutoff) && (j != i)){
                        rowMap[j] = distance;
                    }
                    index++;
                    reading->update(index);
                }
                
                m->gobble(fileHandle);
                
                //save position in file of each new row
                rowPos[i] = out.tellp();
                
                //output row to file
                out << i << '\t' << rowMap.size() << '\t';
                for (itRow = rowMap.begin(); itRow != rowMap.end(); itRow++) {
                    out << itRow->first << '\t' << itRow->second << '\t';
                }
                out << endl;
                
                //clear map for new row's info
                rowMap.clear();
            }
        }
        reading->finish();
        delete reading;
        fileHandle.close();
        out.close();
        
        if (m->control_pressed) { m->mothurRemove(distFile);  return 0; }
        
        list->setLabel("0");
        
        return 1;
        
        
	}
	catch(exception& e) {
        m->errorOut(e, "FormatPhylipMatrix", "read");
        exit(1);
	}
}

/***********************************************************************/
FormatPhylipMatrix::~FormatPhylipMatrix(){}
/***********************************************************************/


