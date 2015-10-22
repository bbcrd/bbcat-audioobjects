#ifndef __TINY_XML_ADM_DATA__
#define __TINY_XML_ADM_DATA__

#include "ADMData.h"

class TiXmlNode;

BBC_AUDIOTOOLBOX_START

//BBC_AUDIOTOOLBOX_KEEP(TinyXMLADMData);
extern const bool keep_TinyXMLADMData;

/*--------------------------------------------------------------------------------*/
/** An implementation of ADMData using the TinyXML library (GPL)
 */
/*--------------------------------------------------------------------------------*/
class TinyXMLADMData : public ADMData
{
public:
  TinyXMLADMData(const std::string& standarddefinitionsfile);
  virtual ~TinyXMLADMData();
  
protected:
  /*--------------------------------------------------------------------------------*/
  /** Register function - this is called automatically
   */
  /*--------------------------------------------------------------------------------*/
  static bool Register();

  /*--------------------------------------------------------------------------------*/
  /** Decode XML string as ADM
   *
   * @param data ptr to string containing ADM XML (MUST be terminated)
   *
   * @return true if XML decoded correctly
   */
  /*--------------------------------------------------------------------------------*/
  virtual bool TranslateXML(const char *data);

  /*--------------------------------------------------------------------------------*/
  /** Parse XML header
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseHeader(ADMHEADER& header, const std::string& type, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse value (and its attributes) into a list of XML values
   *
   * @param obj ADM object
   * @param userdata implementation specific object data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseValue(ADMObject *obj, void *userdata);
  
  /*--------------------------------------------------------------------------------*/
  /** Parse value (and its attributes) into a list of XML values
   *
   * @param name name of object (for DEBUGGING only)
   * @param values list of XML values to be added to
   * @param userdata implementation specific object data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseValue(const std::string& name, XMLValues& values, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse attributes and subnodes as values
   *
   * @param obj object to read values from
   * @param userdata user suppled data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseValues(ADMObject *obj, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse audioBlockFormat XML object
   *
   * @param name parent name (for DEBUGGING only)
   * @param obj audioBlockFormat object
   * @param userdata user suppled data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseValues(const std::string& name, ADMAudioBlockFormat *obj, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse attributes into a list of XML values
   *
   * @param obj ADM object
   * @param userdata implementation specific object data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseAttributes(ADMObject *obj, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Parse attributes into a list of XML values
   *
   * @param name name of object (for DEBUGGING only)
   * @param type object type - necessary to prevent object name and ID being added
   * @param values list of XML values to be populated
   * @param userdata implementation specific object data
   */
  /*--------------------------------------------------------------------------------*/
  virtual void ParseAttributes(const std::string& name, const std::string& type, XMLValues& values, void *userdata);

  /*--------------------------------------------------------------------------------*/
  /** Find named element in subnode list
   *
   * @param node parent node
   * @param name name of subnode to find
   */
  /*--------------------------------------------------------------------------------*/
  virtual const TiXmlNode *FindElement(const TiXmlNode *node, const std::string& name);

  /*--------------------------------------------------------------------------------*/
  /** Recursively collect all objects in tree and parse them
   */
  /*--------------------------------------------------------------------------------*/
  virtual void CollectObjects(const TiXmlNode *node);

  /*--------------------------------------------------------------------------------*/
  /** Creator for this class
   */
  /*--------------------------------------------------------------------------------*/
  static ADMData *__Creator(const std::string& standarddefinitionsfile, void *context)
  {
    UNUSED_PARAMETER(context);
    return new TinyXMLADMData(standarddefinitionsfile);
  }

protected:
  static const bool registered;
};

BBC_AUDIOTOOLBOX_END

#endif
