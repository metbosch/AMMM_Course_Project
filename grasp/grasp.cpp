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

#define min3(_a,_b,_c) (std::min((_a),std::min((_b),(_c))))

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

   unsigned int getDemandOffice( unsigned int id ) {
      return id > demand.size() ? 0 : demand[id];
   }

   unsigned int getCapacityCenter( unsigned int id ) {
      return id > capacity.size() ? 0 : capacity[id];
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

typedef struct relation_str{
   unsigned int officeId;
   std::vector<connection_t> connections;

   relation_str( unsigned int id ) {
      officeId = id;
   }

   relation_str() {}

   unsigned int getCapacityCenter( unsigned int center ) {
      for ( int i = 0; i < connections.size(); ++i ) {
         if ( center == connections[i].centerId ) return connections[i].capacity;
      }
      return 0;
   }

   int getConnectionId( unsigned int center ) {
      for ( int i = 0; i < connections.size(); ++i ) {
         if ( center == connections[i].centerId ) return i;
      }
      return -1;
   }
} relation_t;
typedef relation_t* relation_ptr;

typedef struct {
   unsigned int centerId;
   unsigned int storedPB;
} centerInfo_t;

typedef struct solution_str {
   std::vector< relation_ptr > usage;
   std::vector< unsigned int> accCenters;

   solution_str( inputData_ptr data ) {
      unsigned int offices = data->nOffices;
      unsigned int centers = data->nBackupCenters;
      usage.resize( offices );
      accCenters.resize( centers );
      for ( int center = 0; center < centers; ++center ) {
         accCenters[center] = 0;
      }
      for ( int office = 0; office < offices; ++office ) {
         usage[office] = NULL;
      }
   }

   void addRelation( relation_ptr r, inputData_ptr data ) {
      unsigned int office = r->officeId;
      if ( office < usage.size() ) {
         assert( usage[office] == NULL );
         usage[office] = r;
         for ( int i = 0; i < r->connections.size(); ++i ) {
            unsigned int center = r->connections[i].centerId;
            assert( center < accCenters.size() );
            accCenters[center] += r->connections[i].capacity;
            assert( accCenters[center] <= data->capacity[center] );
         }
      }
   }

   relation_ptr removeRelationOffice( unsigned int id ) {
      assert( id < usage.size() && usage[id] != NULL );
      relation_ptr ret = usage[id];
      usage[id] = NULL;
      for ( int i = 0; i < ret->connections.size(); ++i ) {
         accCenters[ret->connections[i].centerId] -= ret->connections[i].capacity;
      }
      return ret;
   }

   unsigned int getScore( inputData_ptr data ) {
      int ret = 0;
      for ( int center = 0; center < accCenters.size(); ++center ) {
         int tmp = accCenters[center];
         ret += tmp != 0 ? data->getFixedCostCenter( center ) : 0;
         ret += tmp*data->getCostPerPB( tmp );
      }
      return ret;
   }

   int evaluateMove( unsigned int prevCapacity, unsigned int prevCenter, unsigned int newCapacity, unsigned int newCenter, inputData_ptr data ) {
      assert( prevCenter < accCenters.size() && newCenter < accCenters.size() );
      unsigned int tmp = accCenters[prevCenter] + prevCapacity;
      int origScore = data->getFixedCostCenter( prevCenter ) + tmp*data->getCostPerPB( tmp );
      tmp = accCenters[newCenter] + newCapacity;
      origScore += ( tmp != 0 ? data->getFixedCostCenter( newCenter ) : 0 ) + tmp*data->getCostPerPB( tmp );
      tmp = accCenters[prevCenter];
      int newScore = ( tmp != 0 ? data->getFixedCostCenter( prevCenter ) : 0 ) + tmp*data->getCostPerPB( tmp );
      tmp = accCenters[newCenter] + newCapacity + prevCapacity;
      newScore += data->getFixedCostCenter( newCenter ) + tmp*data->getCostPerPB( tmp );
      return origScore - newScore;
   }

   unsigned int getCenterUsage( unsigned int id ) {
      return id > accCenters.size() ? 0 : accCenters[id];
   }

   void print( inputData_ptr data ) {
      std::cout << "------------ SOLUTION ------------" << std::endl;
      for ( int office = 0; office < data->nOffices; ++office ) {
         assert( usage[office] != NULL && usage[office]->officeId == office );
         std::cout << "Office " << office << " (" << data->getDemandOffice( office ) << " PBs):\t[";
         unsigned int total = 0;
         for ( int i = 0; i < usage[office]->connections.size(); ++i ) {
            total += usage[office]->connections[i].capacity;
            std::cout << "{" << usage[office]->connections[i].centerId << ", " << usage[office]->connections[i].capacity << " PBs}";
         }
         std::cout << "] ["  << (( double )( total )/( double )( data->getDemandOffice( office ) ))*100 << "\%]" << std::endl;
      }
      std::cout << "---------------------------------" << std::endl;
      for ( int center = 0; center < accCenters.size(); ++center ) {
         std::cout << "Center " << center << " (" << data->getCapacityCenter( center ) << " PBs):\t[";
         unsigned int total = accCenters[center];
         /*for ( int office = 0; office < data->nOffices; ++office ) {
            if ( usage[center][office] != 0 ) {
               total += usage[center][office];
               std::cout << "{" << office << ", " << usage[center][office] << " PBs}";
            }
         }*/
         std::cout << "] ["  << (( double )( total )/( double )( data->getCapacityCenter( center ) ))*100 << "\%] " << std::endl;
      }
      std::cout << "---------------------------------" << std::endl;
   }
} solution_t;
typedef solution_t* solution_ptr;

typedef struct scoredRelation_str {
   int score;
   unsigned int id;
   relation_ptr relation;

   scoredRelation_str( unsigned int id, int sc ) {
      this->id = id;
      this->score = sc;
      this->relation = NULL;
   }
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
   unsigned int maxCapacity = std::min( inputData->capacity[cId] - s->getCenterUsage( cId ), inputData->getDemandOffice( rel->officeId )*2 ); // NOTE: Increasing the demand bound causes a higer probability of suceed with small amounts of centers
   unsigned int qMinCapacity = min3( rand()%maxCapacity, ( unsigned int )( capacity ), inputData->getDemandOffice( rel->officeId ) );
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

   unsigned int maxCapacity = std::min( inputData->capacity[cId] - s->getCenterUsage( cId ), inputData->getDemandOffice( rel->officeId ) );
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
      assert( rel->connections.size() == 0 );
      ret = qRecursiveRand( rel, s, centerIds, inputData->demand[rel->officeId]*2 ); // We need to map two copies per office ->request two times the demand
   }
   if ( ret >= INF ) {
      assert( rel->connections.size() == 0 );
      ret = qRecursiveMax( rel, s, centerIds, inputData->demand[rel->officeId]*2 );
   }
   //std::cout << "q(" << rel->officeId << ", s) with " << centerIds.size() << " centers and a demand of " << inputData->demand[rel->officeId] << ": [ ";
   //for ( int i = 0 ; i < rel->connections.size(); ++i) std::cout << "{" << rel->connections[i].centerId << ", " << rel->connections[i].capacity << "} ";
   //std::cout << "]: score " << ret << std::endl;
   return ret;
}

solution_ptr constructivePhaseRecursive( double alpha, int depth ) {
   solution_ptr s = NULL;
   if ( depth < MAX_CONSTRUCTIVE_DEPTH ) {
      s = new solution_t( inputData );
      std::vector<scoredRelation_ptr> candidates( inputData->nOffices );
      for ( int i = 0; i < candidates.size(); ++i ) {
         candidates[i] = new scoredRelation_t( i, INF );
      }
      while ( candidates.size() != 0 ) {
         int minScore = INF;
         int maxScore = -INF;
         for ( int i = 0; i < candidates.size(); ++i ) {
            if ( candidates[i]->relation != NULL ) delete candidates[i]->relation;
            candidates[i]->relation = new relation_t( candidates[i]->id );
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
         unsigned int candIdx = rand()%candidates.size();
         s->addRelation( candidates[candIdx]->relation, inputData );
         candidates.erase( candidates.begin() + candIdx );
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
   for ( unsigned int office = 0; office < inputData->nOffices; ++office ) {
      relation_ptr rel = w->removeRelationOffice( office );
      for ( int i = 0; i < rel->connections.size(); ++i ) {
         unsigned int origCenter = rel->connections[i].centerId;
         unsigned int cap = rel->connections[i].capacity;
         unsigned int connectionId = -1;
         unsigned int newCenter = origCenter;
         for ( unsigned int center = 0; center < inputData->nBackupCenters; ++center ) {
            if ( center == origCenter ) continue;
            int connCenterId = rel->getConnectionId( center );
            unsigned int capCenter = connCenterId == -1 ? 0 : rel->connections[connCenterId].capacity;
            if ( inputData->united[office][center] && w->getCenterUsage( center ) + cap + capCenter <= inputData->getCapacityCenter( center ) ) {
               if ( w->evaluateMove( cap, origCenter, capCenter, center, inputData ) > 0 ) {
                  connectionId = connCenterId;
                  newCenter = center;
               }
            }
         }
         if ( connectionId == -1 ) {
            rel->connections[i].centerId = newCenter;
         } else {
            rel->connections[connectionId].capacity += cap;
            rel->connections.erase( rel->connections.begin() + i );
            --i;
         }
      }
      w->addRelation( rel, inputData );
   }
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

   wPlus->print( inputData );
   std::cout << "Solution: " << wPlusScore << std::endl;
   return 0;
}
