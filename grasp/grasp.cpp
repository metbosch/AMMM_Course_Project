#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>     /* assert */
#include <stdlib.h>     /* atoi */

/*****    Constants and types definitions    *****/
#define MAX_ITERS 1
#define ALPHA 0.5
#define INF 999999999

#define DEBUG(_stream) ( std::cout << _stream << std::endl )

typedef struct {
   unsigned int centerId;
   unsigned int capacity;
} connection_t;

typedef struct {
   unsigned int officeId;
   std::vector<connection_t> connections;
} relation_t;
typedef relation_t* relation_ptr;

typedef struct {
   unsigned int centerId;
   unsigned int storedPB;
} centerInfo_t;

typedef struct {
   std::vector<relation_ptr> relations;
   std::vector<unsigned int> centersUsage;
} solution_t;
typedef solution_t* solution_ptr;

typedef struct {
   int score;
   relation_ptr relation;
} scoredRelation_t;
typedef scoredRelation_t* scoredRelation_ptr;

typedef struct {
   unsigned int nOffices;
   unsigned int nBackupCenters;
   unsigned int nSegments;
   std::vector<unsigned int> demand;
   std::vector< std::vector<bool> >united;
   std::vector<unsigned int> capacity;
   std::vector<unsigned int> fixedCost;
   std::vector<unsigned int> costPerPB;
   std::vector<unsigned int> minimumPB;
} inputData_t;
typedef inputData_t* inputData_ptr;
/***** End of constants and types definitions *****/

std::vector<std::string> splitString(const std::string &s, char delim) {
   // http://ysonggit.github.io/coding/2014/12/16/split-a-string-using-c.html
   std::stringstream ss( s );
   std::string item;
   std::vector<std::string> tokens;
   while (std::getline(ss, item, delim)) {
      tokens.push_back(item);
   }
   return tokens;
}

inputData_ptr inputData;
void parseDataFile( const char* file ) {
   inputData = new inputData_t;
   int parsed = 0;
   std::ifstream strmFile;
   strmFile.open( file );
   assert( strmFile.is_open() );
   while ( !strmFile.eof() ) {
      unsigned int count = 0;
      std::string line;
      std::getline( strmFile, line );
      if ( line.find( "nOffices" ) != std::string::npos ) {
         ++parsed;
         /* Read between the " = " and the final ";" */
         int pos = line.find( " = " );
         assert( pos != std::string::npos );
         pos += 3;
         std::string val = line.substr( pos, line.length() - pos - 1 );
         inputData->nOffices = atoi( val.c_str() );
      } else if ( line.find( "nBackupCenters" ) != std::string::npos ) {
         ++parsed;
         /* Read between the " = " and the final ";" */
         int pos = line.find( " = " );
         assert( pos != std::string::npos );
         pos += 3;
         std::string val = line.substr( pos, line.length() - pos - 1 );
         inputData->nBackupCenters = atoi( val.c_str() );
      } else if ( line.find( "nSegments" ) != std::string::npos ) {
         ++parsed;
         /* Read between the " = " and the final ";" */
         int pos = line.find( " = " );
         assert( pos != std::string::npos );
         pos += 3;
         std::string val = line.substr( pos, line.length() - pos - 1 );
         inputData->nSegments = atoi( val.c_str() );
      } else if ( line.find( "demand" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ " );
         int posEnd = line.find( " ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 2, posEnd - posIni - 2 );
         inputData->demand.resize( inputData->nOffices );
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" ) {
               inputData->demand[count] = atoi( vals[i].c_str() );
               ++count;
            }
         }
         assert( count == inputData->nOffices );
      } else if ( line.find( "united" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ [ " );
         int posEnd = line.find( " ] ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 4, posEnd - posIni - 4 );
         inputData->united.resize( inputData->nOffices );
         for ( int i = 0; i < inputData->united.size(); ++i ) {
            inputData->united[i].resize( inputData->nBackupCenters );
         }
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" && vals[i] != "]" && vals[i] != "[" ) {
               inputData->united[count/inputData->nBackupCenters][count%inputData->nBackupCenters] = ( vals[i] == "1" );
               ++count;
            }
         }
         assert( count == inputData->nOffices * inputData->nBackupCenters );
      } else if ( line.find( "capacity" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ " );
         int posEnd = line.find( " ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 2, posEnd - posIni - 2 );
         inputData->capacity.resize( inputData->nBackupCenters );
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" ) {
               inputData->capacity[count] = atoi( vals[i].c_str() );
               ++count;
            }
         }
         assert( count == inputData->nBackupCenters );
      } else if ( line.find( "fixedCost" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ " );
         int posEnd = line.find( " ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 2, posEnd - posIni - 2 );
         inputData->fixedCost.resize( inputData->nBackupCenters );
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" ) {
               inputData->fixedCost[count] = atoi( vals[i].c_str() );
               ++count;
            }
         }
         assert( count == inputData->nBackupCenters );
      } else if ( line.find( "costPerPB" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ " );
         int posEnd = line.find( " ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 2, posEnd - posIni - 2 );
         inputData->costPerPB.resize( inputData->nSegments );
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" ) {
               inputData->costPerPB[count] = atoi( vals[i].c_str() );
               ++count;
            }
         }
         assert( count == inputData->nSegments );
      } else if ( line.find( "minimumPB" ) != std::string::npos ) {
         ++parsed;
         /* Read between the "[ " and the final " ]" */
         int posIni = line.find( "[ " );
         int posEnd = line.find( " ]" );
         assert( posIni != std::string::npos && posEnd != std::string::npos );
         std::string val = line.substr( posIni + 2, posEnd - posIni - 2 );
         inputData->minimumPB.resize( inputData->nSegments );
         std::vector<std::string> vals = splitString( val, ' ' );
         for ( int i = 0; i < vals.size(); ++i ) {
            if ( vals[i] != "" ) {
               inputData->minimumPB[count] = atoi( vals[i].c_str() );
               ++count;
            }
         }
         assert( count == inputData->nSegments );
      }
   }
   assert( parsed == 9 );
   strmFile.close();
}

unsigned int getSegment( unsigned int capacity ) {
   unsigned int ret = inputData->nSegments - 1;
   while ( inputData->minimumPB[ret] > capacity && ret != 0 ) --ret;
   return ret;
}

int f( solution_ptr s ) {
   int ret = 0;
   for ( int id = 0; id < s->centersUsage.size(); ++id ) {
      unsigned int usage = s->centersUsage[id];
      ret += usage == 0 ? inputData->fixedCost[id] : 0;
      ret += usage*inputData->costPerPB[getSegment( usage )];
   }
   return ret;
}

int qRecursive( relation_ptr& rel, solution_ptr s, std::vector<unsigned int>& centerIds, int capacity ) {
   if ( capacity == 0 ) {
      return 0;
   }
   if ( centerIds.size() == 0 ) {
      return INF;
   }
   unsigned int cId = centerIds[centerIds.size() - 1];
   centerIds.pop_back();
   relation_ptr qMinRel = new relation_t;
   qMinRel->officeId = rel->officeId;
   int qMin = qRecursive( qMinRel, s, centerIds, capacity );
   unsigned int qMinCapacity = 0;
   for ( int c = capacity; c > 0; c = c/2 ) {
      if ( s->centersUsage[cId] + c > inputData->capacity[cId] ) continue;
      if ( c > inputData->demand[rel->officeId] ) continue;
      if ( s->centersUsage[cId] + c < inputData->minimumPB[0] ) break;
      relation_ptr tmpRel = new relation_t; tmpRel->officeId = rel->officeId;
      unsigned int segmentPrev = getSegment( s->centersUsage[cId] );
      unsigned int segmentNew = getSegment( s->centersUsage[cId] + c );
      int qNew = s->centersUsage[cId] == 0 ? inputData->fixedCost[cId] : 0;
      qNew += c*inputData->costPerPB[segmentNew];
      qNew += ( inputData->costPerPB[segmentNew] - inputData->costPerPB[segmentPrev] )*s->centersUsage[cId];
      qNew += qRecursive( tmpRel, s, centerIds, capacity - c );
      if ( qNew < qMin ) {
         qMin = qNew;
         qMinCapacity = c;
         delete qMinRel;
         qMinRel = tmpRel;
      } else {
         delete tmpRel;
      }
   }
   centerIds.push_back( cId );
   if ( qMinCapacity > 0 ) {
      connection_t tmp;
      tmp.centerId = cId;
      tmp.capacity = qMinCapacity;
      qMinRel->connections.push_back( tmp );
      delete rel;
      rel = qMinRel;
   }
   return qMin;
}

int q( relation_ptr rel, solution_ptr s ) {
   // Filtrar centres si estan conectats
   std::vector<unsigned int> centerIds;
   for ( int i = 0; i < inputData->nBackupCenters; ++i ) {
      if ( inputData->united[rel->officeId][i] && s->centersUsage[i] < inputData->capacity[i] ) {
         centerIds.push_back( i );
      }
   }
   relation_ptr tmpRel = new relation_t; tmpRel->officeId = rel->officeId;
   int ret = qRecursive( tmpRel, s, centerIds, inputData->demand[rel->officeId]*2 ); // We need to map two copies per office ->request two times the demand
   rel->connections.insert( rel->connections.begin(), tmpRel->connections.begin(), tmpRel->connections.end() );
   delete tmpRel;
   std::cout << "q(" << rel->officeId << ", s) with " << centerIds.size() << " centers and a demand of " << inputData->demand[rel->officeId] << ": [ ";
   for ( int i = 0 ; i < rel->connections.size(); ++i) std::cout << "{" << rel->connections[i].centerId << ", " << rel->connections[i].capacity << "} ";
   std::cout << "]" << std::endl;
   return ret;
}

solution_ptr newEmptySolution() {
   solution_ptr s = new solution_t;
   s->centersUsage.resize( inputData->nBackupCenters );
   for ( int i = 0; i < s->centersUsage.size(); ++i ) {
      s->centersUsage[i] = 0;
   }
   return s;
}

void addRelationToSolution( relation_ptr rel, solution_ptr s ) {
   for ( int i = 0; i < rel->connections.size(); ++i ) {
      int id = rel->connections[i].centerId;
      s->centersUsage[id] += rel->connections[i].capacity;
      //DEBUG( "office " << rel->officeId << " - " << i << " > center " << id << ": " << rel->connections[i].capacity << " PBs, total used " <<  s->centersUsage[id]);
      assert( s->centersUsage[id] <= inputData->capacity[id] );
   }
   s->relations.push_back( rel );
}

solution_ptr constructivePhase( double alpha ) {
   solution_ptr s = newEmptySolution();
   std::vector<scoredRelation_ptr> candidates( inputData->nOffices );
   for ( int i = 0; i < candidates.size(); ++i ) {
      candidates[i] = new scoredRelation_t;
      candidates[i]->score = INF;
      candidates[i]->relation = new relation_t;
      candidates[i]->relation->officeId = i; // This should be moved
   }
   while ( candidates.size() != 0 ) {
      int minScore = INF;
      int maxScore = -INF;
      for ( int i = 0; i < candidates.size(); ++i ) {
         candidates[i]->score = q( candidates[i]->relation, s );
         assert( candidates[i]->score != INF );
         minScore = std::min( candidates[i]->score, minScore );
         maxScore = std::max( candidates[i]->score, maxScore );
      }
      int rclThreshold = maxScore + alpha*( minScore - maxScore );
      std::vector<scoredRelation_ptr> rcl;
      for ( int i = 0; i < candidates.size(); ++i ) {
         if ( candidates[i]->score <= rclThreshold ) {
            rcl.push_back( candidates[i] );
         }
      }
      scoredRelation_ptr rel = candidates[rand()%candidates.size()];
      addRelationToSolution( rel->relation, s );
      for ( int i = 0; i < candidates.size(); ++i ) {
         if ( rel == candidates[i] ) {
            candidates.erase( candidates.begin() + i );
            break;
         }
      }
   }

   return s;
}

solution_ptr localSearchPhase( solution_ptr w ) {
   return w;
}

int main( int argc, char** argv ) {
   int maxIters = argc < 4 ? MAX_ITERS : atoi( argv[3] );
   double alpha = argc < 3 ? ALPHA : atof( argv[2] );
   const char* dataFile = argc < 2 ? "input.dat" : argv[1];

   DEBUG( "Parsing input data file" );
   parseDataFile( dataFile );

   DEBUG( "Starting main loop" );
   solution_ptr wPlus = NULL;
   int wPlusScore = INF;
   for ( int iter = 0; iter < maxIters; ++iter ) {
      DEBUG( "Main loop iteration: " << iter );
      solution_ptr w = constructivePhase( alpha );
      w = localSearchPhase( w );
      int wScore = f( w );
      if ( wScore < wPlusScore ) {
         delete wPlus;
         wPlus = w;
         wPlusScore = wScore;
      }
   }

   std::cout << "Solution: " << wPlusScore << std::endl;
   return 0;
}
