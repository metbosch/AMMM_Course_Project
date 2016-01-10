#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>     /* assert */
#include <stdlib.h>     /* atoi */

/*****    Constants and types definitions    *****/
#define MAX_ITERS 100
#define ALPHA 0.5
#define INF 999999999
#define MAX_CONSTRUCTIVE_DEPTH 100

//#define DEBUG(_stream) ( std::cout << _stream << std::endl )
#define DEBUG(_stream)
#define VERBOSE(_stream) ( std::cout << _stream << std::endl )

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

   unsigned int getSegment( unsigned int capacity ) {
      unsigned int ret = nSegments - 1;
      while ( minimumPB[ret] > capacity && ret != 0 ) --ret;
      return ret;
   }

   unsigned int getCostPerPB( unsigned int usage ) {
      return costPerPB[getSegment( usage )];
   }

   unsigned int getFixedCostCenter( unsigned int id ) {
      return id > fixedCost.size() ? INF : fixedCost[id];
   }
} inputData_t;
typedef inputData_t* inputData_ptr;

typedef struct connection_str {
   unsigned int centerId;
   unsigned int capacity;
public:
   connection_str( unsigned int id, unsigned int c ) {
      this->centerId = id;
      this->capacity = c;
   }
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

typedef struct solution_str {
   std::vector< std::vector<unsigned int> > usage;
   std::vector< unsigned int> accCenters;

   solution_str( inputData_ptr data ) {
      unsigned int offices = data->nOffices;
      unsigned int centers = data->nBackupCenters;
      usage.resize( centers );
      accCenters.resize( centers );
      for ( int i = 0; i < centers; ++i ) {
         accCenters[i] = 0;
         usage[i].resize( offices );
         for ( int j = 0; j < offices; ++j ) {
            usage[i][j] = 0;
         }
      }
   }

   void addRelation( relation_ptr r ) {
      unsigned int office = r->officeId;
      for ( int i = 0; i < r->connections.size(); ++i ) {
         unsigned int center = r->connections[i].centerId;
         if ( center < usage.size() && office < usage[center].size() ) {
            usage[center][office] += r->connections[i].capacity;
            accCenters[center] += r->connections[i].capacity;
         }
      }
   }

   unsigned int getScore( inputData_ptr data ) {
      int ret = 0;
      for ( int center = 0; center < usage.size(); ++center ) {
         int usage = accCenters[center];
         /*for ( int office = 0; office < usage[center].size(); ++office ) {
            usage += usage[center][office];
         }*/
         ret += usage != 0 ? data->getFixedCostCenter( center ) : 0;
         ret += usage*data->getCostPerPB( usage );
      }
      return ret;
   }

   unsigned int getCenterUsage( unsigned int id ) {
      return id > usage.size() ? 0 : accCenters[id];
   }
} solution_t;
typedef solution_t* solution_ptr;

typedef struct {
   int score;
   relation_ptr relation;
} scoredRelation_t;
typedef scoredRelation_t* scoredRelation_ptr;
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

int qRecursiveRand( relation_ptr rel, solution_ptr s, std::vector<unsigned int>& centerIds, int capacity ) {
   if ( capacity <= 0 ) {
      return 0;
   }
   if ( centerIds.size() == 0 ) {
      DEBUG( "qRecursiveRand: no centers available and capacity is " << capacity << " for office " << rel->officeId );
      return INF;
   }
   int tmp = rand()%centerIds.size();
   unsigned int cId = centerIds[tmp];
   centerIds.erase( centerIds.begin() + tmp );

   // Try a Random capacity
   unsigned int maxCapacity = inputData->capacity[cId] - s->getCenterUsage( cId );
   unsigned int qMinCapacity = std::min( rand()%maxCapacity, ( unsigned int )( capacity ) );
   unsigned int costSegPrev = inputData->getCostPerPB( s->getCenterUsage( cId ) );
   unsigned int costSegNew = inputData->getCostPerPB( s->getCenterUsage( cId ) + qMinCapacity );
   int qMin = s->getCenterUsage( cId ) == 0 ? inputData->getFixedCostCenter( cId ) : 0;
   int qRec = qRecursiveRand( rel, s, centerIds, capacity - qMinCapacity );
   qMin += qMinCapacity*costSegNew;
   qMin += ( costSegNew - costSegPrev )*s->getCenterUsage( cId );
   qMin = qRec >= INF ? INF : qMin + qRec;
   DEBUG( "qRecursiveRand: center " << cId << " using " << qMinCapacity << " of " << maxCapacity << " (" <<inputData->capacity[cId]<< ", " << s->getCenterUsage( cId ) << ", " <<inputData->demand[rel->officeId]<<  ")" );

   centerIds.push_back( cId );
   if ( qMinCapacity > 0 && qMin < INF ) {
      rel->connections.push_back( connection_t( cId, qMinCapacity ) );
   }
   return qMin;
}

int qRecursiveMax( relation_ptr rel, solution_ptr s, std::vector<unsigned int>& centerIds, int capacity ) {
   if ( capacity <= 0 ) {
      return 0;
   }
   if ( centerIds.size() == 0 ) {
      DEBUG( "qRecursiveMax: no centers available and capacity is " << capacity << " for office " << rel->officeId );
      return INF;
   }
   int tmp = rand()%centerIds.size();
   unsigned int cId = centerIds[tmp];
   centerIds.erase( centerIds.begin() + tmp );

   unsigned int maxCapacity = inputData->capacity[cId] - s->getCenterUsage( cId );
   unsigned int qMinCapacity = std::min( maxCapacity, ( unsigned int )( capacity ) );
   unsigned int costSegPrev = inputData->getCostPerPB( s->getCenterUsage( cId ) );
   unsigned int costSegNew = inputData->getCostPerPB( s->getCenterUsage( cId ) + qMinCapacity );
   DEBUG( "qRecursiveMax: center " << cId << " using " << qMinCapacity << " of " << maxCapacity << " (" <<inputData->capacity[cId]<< ", " << s->getCenterUsage( cId ) << ", " <<inputData->demand[rel->officeId]<<  ")" );
   int qMin = s->getCenterUsage( cId ) == 0 ? inputData->getFixedCostCenter( cId ) : 0;
   qMin += qMinCapacity*costSegNew;
   qMin += ( costSegNew - costSegPrev )*s->getCenterUsage( cId );
   int qRec = qRecursiveMax( rel, s, centerIds, capacity - qMinCapacity );
   qMin = qRec >= INF ? INF : qMin + qRec;

   centerIds.push_back( cId );
   if ( qMinCapacity > 0 && qMin < INF ) {
      rel->connections.push_back( connection_t ( cId, qMinCapacity ) );
   }
   return qMin;
}

int q( relation_ptr& rel, solution_ptr s ) {
   // Filtrar centres si estan conectats
   std::vector<unsigned int> centerIds;
   unsigned int availableCapacity = 0;
   for ( int i = 0; i < inputData->nBackupCenters; ++i ) {
      if ( inputData->united[rel->officeId][i] && s->getCenterUsage( i ) < inputData->capacity[i] ) {
         centerIds.push_back( i );
         availableCapacity += std::min( inputData->capacity[i] - s->getCenterUsage( i ), inputData->demand[rel->officeId] );
      }
   }
   if ( availableCapacity < inputData->demand[rel->officeId]*2 ) return INF;
   int maxTries = 10;
   int ret = INF;
   for ( int tries = 0; tries < maxTries && ret >= INF; ++tries ) {
      ret = qRecursiveRand( rel, s, centerIds, inputData->demand[rel->officeId]*2 ); // We need to map two copies per office ->request two times the demand
      //std::cout << "q(" << rel->officeId << ", s) with " << centerIds.size() << " centers and a demand of " << inputData->demand[rel->officeId] << ": [ ";
      //for ( int i = 0 ; i < rel->connections.size(); ++i) std::cout << "{" << rel->connections[i].centerId << ", " << rel->connections[i].capacity << "} ";
      //std::cout << "]: score " << ret << std::endl;
   }
   if ( ret >= INF ) {
      ret = qRecursiveMax( rel, s, centerIds, inputData->demand[rel->officeId]*2 );
   }
   return ret;
}

solution_ptr constructivePhaseRecursive( double alpha, int depth ) {
   VERBOSE( "Recursive constructive phase, depth " << depth );
   solution_ptr s = NULL;
   if ( depth < MAX_CONSTRUCTIVE_DEPTH ) {
      s = new solution_t( inputData );
      std::vector<scoredRelation_ptr> candidates( inputData->nOffices );
      for ( int i = 0; i < candidates.size(); ++i ) {
         candidates[i] = new scoredRelation_t;
      }
      while ( candidates.size() != 0 ) {
         int minScore = INF;
         int maxScore = -INF;
         for ( int i = 0; i < candidates.size(); ++i ) {
            candidates[i]->relation = new relation_t;
            candidates[i]->relation->officeId = i; // This should be moved
            candidates[i]->score = q( candidates[i]->relation, s );
            if ( candidates[i]->score >= INF ) {
               delete s;
               for ( int i = 0; i < candidates.size(); ++i ) {
                  delete candidates[i];
               }
               return constructivePhaseRecursive( alpha, depth + 1 );
            }
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
         s->addRelation( rel->relation );
         for ( int i = 0; i < candidates.size(); ++i ) {
            if ( rel == candidates[i] ) {
               candidates.erase( candidates.begin() + i );
            } else {
               delete candidates[i]->relation;
            }
         }
      }
      for ( int i = 0; i < candidates.size(); ++i ) {
         delete candidates[i];
      }
   }
   return s;
}

solution_ptr constructivePhase( double alpha ) {
   return constructivePhaseRecursive( alpha, 0 );
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
      int wScore = w->getScore( inputData );
      if ( wScore < wPlusScore ) {
         VERBOSE( "New opt value: " << wScore << " < " << wPlusScore );
         delete wPlus;
         wPlus = w;
         wPlusScore = wScore;
      } else {
         delete w;
      }
   }

   std::cout << "Solution: " << wPlusScore << std::endl;
   return 0;
}
