#ifndef XSD_ERROR_HANDLER
#define XSD_ERROR_HANDLER

#include "jass.hxx"

   /**
        This error handler can be used in conjunction with
        the generated serialization/deserialization methods
        to create C++ types from XML documents
    */
    class xsd_error_handler : public xml_schema::error_handler
    {
        virtual bool handle (
            const std::string& id,
            unsigned long line,
            unsigned long column,
            severity,
            const std::string& message
        ) {
                std::cerr
                    << "[ParsingErrorHandler]: \""
                    << message <<"\""
                    << " in line: " << line
                    << ", column: " << column
                    << std::endl;

                return false;
            }
    };

#endif