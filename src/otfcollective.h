#ifndef OTFCOLLECTIVE_H
#define OTFCOLLECTIVE_H

#include <string>

class OTFCollective
{
public:
    OTFCollective(int _id, int _type, std::string _name);

    int id;
    int type;
    std::string name;
};

#endif // OTFCOLLECTIVE_H
