#include "Newspapers/TMOPNewspaperItemDefinition.h"

UTMOPNewspaperItemDefinition::UTMOPNewspaperItemDefinition()
{
    ItemType = ETMOPItemType::Newspaper;
    MaximumStack = 1;
    bCanDrop = true;
    bCanEquip = true;
    bOpensMenuInsteadOfEquipping = true;
}
