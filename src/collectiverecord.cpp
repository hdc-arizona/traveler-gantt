#include "collectiverecord.h"
#include "commevent.h"
#include "collectiveevent.h"

CollectiveRecord::CollectiveRecord(unsigned long long _matching,
                                   unsigned int _root,
                                   unsigned int _collective,
                                   unsigned int _entitygroup)
    : matchingId(_matching),
      root(_root),
      collective(_collective),
      entitygroup(_entitygroup),
      mark(false),
      events(new std::vector<CollectiveEvent *>())

{
}


CommEvent * CollectiveRecord::getDesignee()
{
    return events->front();
}
