#pragma once

#include <string>
#include <boost/lexical_cast.hpp>
#include <rapidxml.hpp>

#include <core/common/exception.hpp>

namespace eMU
{
namespace core
{
namespace common
{

class XmlReader
{
public:
    class EmptyXmlContentException: public Exception {};
    class NotMatchedXmlNodeException: public Exception {};

    XmlReader(const std::string &content);

    void parse(std::string firstNodeName);
    bool end();
    void next();
    void clear();

    template<typename DataType>
    DataType get(const std::string &nodeName, const std::string &attributeName, const DataType &defaultValue = DataType())
    {
        if(this->end())
        {
            return defaultValue;
        }

        if(currentNode_->name() != nodeName)
        {
            return defaultValue;
        }

        for (rapidxml::xml_attribute<> *attribute = currentNode_->first_attribute(); attribute != nullptr; attribute = attribute->next_attribute())
        {
            if(attribute->name() == attributeName)
            {
                return boost::lexical_cast<DataType>(attribute->value());
            }
        }

        return defaultValue;
    }

    static std::string getXmlFileContent(const std::string &filePath);

private:
    XmlReader();

    std::string content_;
    rapidxml::xml_document<> document_;
    rapidxml::xml_node<> *currentNode_;
};

}
}
}
