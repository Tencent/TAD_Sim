//
// Created by Jens Klimke on 2019-04-26.
//

#ifndef XSD2CPP_XSD_PARSER_H
#define XSD2CPP_XSD_PARSER_H

#include "DataType.h"

namespace xsd2cpp {

    class XSDParser {

    public:


        static void parseRestrictedType(const tinyxml2::XMLElement *elem, const std::string &name) {

            // create data type
            auto tp = (DataType_Simple *) DataType::dataTypes.at(name);
            tp->type = name;
            tp->originalType = elem->Attribute("base");

            // initialize
            tp->init();

        }


        static void parseUnionType(const tinyxml2::XMLElement *elem, const std::string &name) {

            // create data type
            auto tp = (DataType_Simple *) DataType::dataTypes.at(name);
            tp->type = name;
            tp->originalType = "xsd:string";

            // initialize
            tp->init();

        }


        static void parseAttribute(const tinyxml2::XMLElement *attr, DataType_Complex *tp) {

            auto an = std::string(attr->Attribute("name"));
            auto at = std::string(attr->Attribute("type"));

            // create data field
            auto df = new DataField;

            // create types
            auto pre = at.substr(0, 3);
            if (pre == "xsd:") {
                df->dataType = new DataType_Basic;
                df->dataType->originalType = at;
                df->dataType->init();
            } else
                df->dataType = DataType::dataTypes.at(at);

            // set name
            df->fieldName = "_" + an; //"_f__" + an;
            df->originalName = an;

            // add to list
            tp->attr[df->fieldName] = df;

        }


        static void parseSubElement(const tinyxml2::XMLElement *sub, DataType_Complex *tp, bool multiple = true) {

            // get name and type
            auto an = std::string(sub->Attribute("name"));
            auto at = std::string(sub->Attribute("type") ? sub->Attribute("type") : "xsd:string");

            // get bounds
            std::string min = "0";
            if (sub->Attribute("minOccurs") != nullptr)
                min = std::string(sub->Attribute("minOccurs"));

            std::string max = "1";
            if (sub->Attribute("maxOccurs") != nullptr)
                max = std::string(sub->Attribute("maxOccurs"));

            // unset bounds, when not a potential vector
            if (!multiple) {
                min = "0";
                max = "1";
            }

            // create data field
            auto df = new DataField;
            df->fieldName = "sub_" + an; //"_s__" + an;
            df->originalName = an;
            df->dataType = DataType::dataTypes.at(at);

            // save vector flag
            df->vector = max == "unbounded";

            // add to list
            tp->subs[df->fieldName] = df;

        }

        static void parseGroup(const std::string& name, DataType_Complex *tp, const tinyxml2::XMLElement *root) {
            auto group = root->FirstChildElement("xsd:group");
            while (group != nullptr) {
                if (group->Attribute("name") != nullptr && std::string(group->Attribute("name")) == name){                
                    parseComplexTypeMember(group, tp, root);
                    return;
                }
                group = group->NextSiblingElement("xsd:group");
            }
        }

        static void parseComplexTypeMember(const tinyxml2::XMLElement *elem, DataType_Complex *tp, const tinyxml2::XMLElement *root){

            
            // get members
            auto seq = elem->FirstChildElement("xsd:sequence");
            if (seq != nullptr) {

                auto sub = seq->FirstChildElement("xsd:element");
                while (sub != nullptr) {
                    // parse element
                    parseSubElement(sub, tp, true);
                    sub = sub->NextSiblingElement("xsd:element");
                }

                auto ch = seq->FirstChildElement("xsd:choice");
                while (ch != nullptr) {

                    sub = ch->FirstChildElement("xsd:element");
                    while (sub != nullptr) {

                        // parse element
                        parseSubElement(sub, tp, true);
                        sub = sub->NextSiblingElement("xsd:element");

                    }

                    // next element
                    ch = ch->NextSiblingElement("xsd:element");

                }

                auto gp = seq->FirstChildElement("xsd:group");
                while (gp != nullptr) {
                    parseGroup(gp->Attribute("ref"), tp, root);
                    gp = gp->NextSiblingElement("xsd:group");
                }

            }


            auto ch = elem->FirstChildElement("xsd:choice");
            while (ch != nullptr) {

                auto sub = ch->FirstChildElement("xsd:element");
                while (sub != nullptr) {

                    // parse element
                    parseSubElement(sub, tp, false);
                    sub = sub->NextSiblingElement("xsd:element");

                }

                
                auto gp = ch->FirstChildElement("xsd:group");
                while (gp != nullptr) {
                    parseGroup(gp->Attribute("ref"), tp, root);
                    gp = gp->NextSiblingElement("xsd:group");
                }

                // next element
                ch = ch->NextSiblingElement("xsd:choice");

            }

			auto all = elem->FirstChildElement("xsd:all");
			if (all != nullptr) {
			    auto sub = all->FirstChildElement("xsd:element");
                while (sub != nullptr) {

                    // parse element
                    parseSubElement(sub, tp, false);
                    sub = sub->NextSiblingElement("xsd:element");

                }
                
                auto gp = all->FirstChildElement("xsd:group");
                while (gp != nullptr) {
                    parseGroup(gp->Attribute("ref"), tp, root);
                    gp = gp->NextSiblingElement("xsd:group");
                }
			}

        }

        static void parseComplexType(const tinyxml2::XMLElement *elem, const std::string &name, const tinyxml2::XMLElement *root) {

            // create data type
            auto tp = (DataType_Complex *) DataType::dataTypes.at(name);
            tp->originalType = name;

            // complex content
            auto cc = elem->FirstChildElement("xsd:complexContent");
            if (cc != nullptr) {

                // get super class
                auto ext = cc->FirstChildElement("xsd:extension");
                auto clazz = DataType::dataTypes.at(ext->Attribute("base"));

                // save super class
                tp->superType = clazz->type;

                // get members
                auto attr = ext->FirstChildElement("xsd:attribute");
                while (attr != nullptr) {

                    // parse attribute
                    parseAttribute(attr, tp);

                    // next element
                    attr = attr->NextSiblingElement("xsd:attribute");

                }


            }


            // get members
            auto attr = elem->FirstChildElement("xsd:attribute");
            while (attr != nullptr) {

                // parse attribute
                parseAttribute(attr, tp);

                // next element
                attr = attr->NextSiblingElement("xsd:attribute");

            }
            parseComplexTypeMember(elem, tp, root);
            
            // initialize
            tp->init();

        }


        static void parseTypes(const tinyxml2::XMLElement *elem, const std::string &superName, const tinyxml2::XMLElement *root) {

            auto st = elem->FirstChildElement("xsd:simpleType");
            while (st != nullptr) {

                // get name and create struct
                auto name = std::string(st->Attribute("name"));

                if (!superName.empty())
                    name = superName + "_" + name;

                // check if restriction
                auto res = st->FirstChildElement("xsd:restriction");
                if (res != nullptr)
                    parseRestrictedType(res, name);

                // get next one
                st = st->NextSiblingElement("xsd:simpleType");

            }


            st = elem->FirstChildElement("xsd:simpleType");
            while (st != nullptr) {

                // get name and create struct
                auto name = std::string(st->Attribute("name"));

                if (!superName.empty())
                    name = superName + "_" + name;

                // check if union
                auto un = st->FirstChildElement("xsd:union");
                if (un != nullptr)
                    parseUnionType(un, name);

                // get next one
                st = st->NextSiblingElement("xsd:simpleType");

            }


            // iterate over complex types to process
            st = elem->FirstChildElement("xsd:element");
            while (st != nullptr) {

                // get name and create struct
                auto name = std::string(st->Attribute("name"));

                if (!superName.empty())
                    name = superName + "_" + name;

                parseTypes(st, name, root);

                // get next one
                st = st->NextSiblingElement("xsd:element");

            }


            // iterate over complex types to process
            st = elem->FirstChildElement("xsd:complexType");
            while (st != nullptr) {

                // get name and create struct
                std::string name;

                // get super name
                if (st->Attribute("name") == nullptr)
                    name = superName;
                else if (!superName.empty())
                    name = superName + "_" + name;
                else
                    name = st->Attribute("name");

                // parse super type
                parseComplexType(st, name, root);

                // get next one
                st = st->NextSiblingElement("xsd:complexType");

            }


        }


        static void createTypes(const tinyxml2::XMLElement *elem, const std::string &superName) {

            auto st = elem->FirstChildElement("xsd:simpleType");
            while (st != nullptr) {

                // get name and create struct
                auto ctName = st->Attribute("name");

                // check if restriction
                auto res = st->FirstChildElement("xsd:restriction");
                if (res != nullptr) {

                    // create data type
                    DataType::addDataType(ctName, new DataType_Simple);

                }

                // get next one
                st = st->NextSiblingElement("xsd:simpleType");

            }


            st = elem->FirstChildElement("xsd:simpleType");
            while (st != nullptr) {

                // get name and create struct
                auto ctName = st->Attribute("name");

                // check if union
                auto un = st->FirstChildElement("xsd:union");
                if (un != nullptr) {

                    // create data type
                    DataType::addDataType(ctName, new DataType_Simple);

                }

                // get next one
                st = st->NextSiblingElement("xsd:simpleType");

            }


            // iterate over complex types to create
            st = elem->FirstChildElement("xsd:complexType");
            while (st != nullptr) {

                // get name and create struct
                auto ctName = st->Attribute("name");

                if (ctName == nullptr)
                    ctName = superName.c_str();

                DataType::addDataType(ctName, new DataType_Complex);

                st = st->NextSiblingElement("xsd:complexType");

            }


            // iterate over complex types to process
            st = elem->FirstChildElement("xsd:element");
            while (st != nullptr) {
            
                // get name and create struct
                createTypes(st, st->Attribute("name"));
            
                // get next one
                st = st->NextSiblingElement("xsd:element");
            
            }

        }


        static void parseScheme(const tinyxml2::XMLElement *elem) {

            createTypes(elem, "");
            parseTypes(elem, "", elem);

        }

    };


}


#endif // XSD2CPP_XSD_PARSER_H
