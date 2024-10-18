/**
 * @abstractor.cpp
 * @author Elvan Karasu
 *
 * @brief Multithreaded local search engine. For the given query, the program searchs
 * for the best matching abstracts of scientific articles.
 *
 * The program takes two arguments: the path of input file and the path of output file.
 * It tokenizes the abstract text and calculates the Jaccard similarity between the
 * abstract and the given query. For T threads, it searches over A articles and returns
 * the N best matching articles and their summaries.
 *
 * POSIX thread (pthread) library is used for designing this multithreaded program.
 * Mutex locks are used for handling thread synchronization issues.
 *
 * How to compile and run:
 *    g++ abstractor.cpp -o abstractor.out -lpthread
 *   ./abstractor.out <input_path> <output_path>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <sstream>
#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;
//Defining the global variables accessed by all threads
string query;
set <string> wordsToQuery; //set of words to query
int idx=0; //index to keep track of which abstracts are processed
int A; //number of abstracts to be processed
queue<string> abstractList;
double finalScores[100];
int calculated=0;
int size;
ofstream outputFile;
string outputFileName;
pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

set<string> findIntersection(set<string> s1, set<string> s2) {
    set<string> intersection;
    set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(), inserter(intersection, intersection.begin()));
    return intersection;
}

double findSimilarity(set<string> s1, set<string> s2) {
    double intersectionSize = findIntersection(s1, s2).size();
    double unionSize = s1.size() + s2.size() - intersectionSize;
    double similarity = (double)(intersectionSize/unionSize);
    return similarity;
}

double abstractProcessor(string abstractName) {
    ifstream abstractFile;
    abstractFile.open(abstractName);
    string word, text;
    set <string> abstractWords;

    while(abstractFile >> word) {
        abstractWords.insert(word); //Forming the set of abstract words to calculate similarity
        text += word; //Forming the abstract text
        text.push_back(' ');
    }

    double similarity = findSimilarity(wordsToQuery, abstractWords);
    return similarity;
}

void* runner(void *param) { //runner function that is ran by threads
    string nameOfThread = *(string*) param;

    while (!abstractList.empty()) {

        pthread_mutex_lock(&myMutex); //mutex locked before entering to critical section
        string abstractName = abstractList.front();
        calculated++;
        abstractList.pop();
        pthread_mutex_unlock(&myMutex); //mutex unlocked while exiting critical section

        double sim = abstractProcessor(abstractName);

        pthread_mutex_lock(&myMutex); //mutex locked before entering to critical section
        chdir("../src");
        outputFile.open(outputFileName, std::ios_base::app);
        //outputFile << endl << endl << "This is abstract name: " << abstractName << "This is abstarct name.***" << endl << endl;
        int abstractNum = stoi(abstractName.substr(9, abstractName.length() - 13));
        finalScores[abstractNum] = sim;

        //cout << "Thread " << nameOfThread << " is calculating " << abstractName << endl;
        outputFile << "Thread " << nameOfThread << " is calculating " << abstractName << endl;
        outputFile.close();
        chdir("../abstracts");
        pthread_mutex_unlock(&myMutex); //mutex unlocked while exiting critical section
        /*if (calculated>=size) {
            break;
        }*/
    }

    pthread_exit(NULL); //thread exits the runner function
}

int main(int argc, char const *argv[]) {

    //Reading data from input file
    ifstream inputFile;
    //ofstream outputFile;
    outputFileName = argv[2];
    inputFile.open(argv[1]);
    int T, N; // T: # threads, A: # abstracts to search
    inputFile >> T >> A >> N; // N: # abstracts to find the best matching results (global variable defined above)
    //vector<struct abstract> *abstractList; //vector of abstracts

    string str;
    vector<string> lines;
    while (getline(inputFile, str)) { //Reading lines from the input file
        lines.push_back(str);
    }

    query =  lines.at(1); //First line of the input file is query
    stringstream check(query);
    string token;
    while(getline(check, token, ' ')) { //Tokenizing words to query
        wordsToQuery.insert(token);
    }

    for(int itr=2; itr<=A+1; itr++) {
        abstractList.push((lines.at(itr)));
    }

    size = abstractList.size();

    chdir("../abstracts"); //Changing directory to abstracts file in order to read the abstracts

    for(int i = 0; i < 100; ++i) {
        finalScores[i] = -1;
    }
    pthread_t threads[T];
    string threadNames[T];

    for( int i = 0; i < T; i++ ) {
        threadNames[i] = 65 + i;
        pthread_create(&threads[i], NULL, runner, (void *) &threadNames[i]);
    }

    for( int i = 0; i < T; i++ ) {
        pthread_join(threads[i], NULL);
    }
    priority_queue<double> yourHeap;

    for(int i=0; i<100; i++) {
        if (finalScores[i]>=0) {
            yourHeap.push(finalScores[i]);
        }
    }

    //Writing results to output file
    //cout << "###" << endl;
    chdir("../src");
    outputFile.open (argv[2], fstream::app);
    outputFile << "###" << endl;
    outputFile.close();

    int resultNo = 1;
    string abstractName = "   ";
    for(int itr=0; itr<N; itr++) { //Abstracts are sorted according to their similarity scores
        if (resultNo>N) {
            break;
        }
        for(int j=0; j<100; j++) {
            if (resultNo>N) {
                break;
            }
            if (finalScores[j]==yourHeap.top()) { //search for the top element of the heap
                abstractName =  "abstract_" + to_string(j) + ".txt";
                if (resultNo>N) {
                    break;
                }
                //cout << "Result " << resultNo << ":\n" << "File: " << abstractName << endl;
                //cout << "Score: " << std::setprecision(3) << yourHeap.top() << endl;
                chdir("../src");
                outputFile.open (argv[2], fstream::app);
                outputFile << "Result " << resultNo << ":\n" << "File: " << abstractName << endl;
                outputFile << std::fixed;
                outputFile << "Score: " << std::setprecision(4) << yourHeap.top() << endl;
                outputFile.close();

                resultNo++;
                string abstractStr;
                string abstractText;

                chdir("../abstracts");
                ifstream abstractFile;
                abstractFile.open(abstractName);

                while (getline(abstractFile, abstractStr)) { //Reading lines from the input file
                    abstractText += abstractStr;
                }

                vector<string> sentences;
                stringstream check(abstractText);
                string token;
                while(getline(check, token, '.')) { //Tokenizing words to query
                    sentences.push_back(token);
                }

                //cout << "Summary:";
                chdir("../src");
                outputFile.open (argv[2], fstream::app);
                outputFile << "Summary:";
                for(int k=0; k<sentences.size(); k++) {
                    bool isThereACommonWord = false;
                    for (auto m:wordsToQuery) {
                        if (sentences.at(k).find(m)!=std::string::npos) {

                            isThereACommonWord =true;
                        }
                    }
                    if (isThereACommonWord) {
                        //cout << sentences.at(k) << ".";
                        outputFile << sentences.at(k) << ".";
                    }
                }
                //cout << endl << "###" << endl;
                outputFile << endl << "###" << endl;
                outputFile.close();
                yourHeap.pop();

            };
        }

    }
    inputFile.close();
    outputFile.close();
    return 0;
}