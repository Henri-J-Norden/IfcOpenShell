#include "IfcSchema.h"
#include "../ifcparse/IfcBaseClass.h"

#include <map>

bool IfcParse::declaration::is(const std::string& name) const {
	if (name_lower_ == boost::to_lower_copy(name)) return true;

	if (this->as_entity()) {
		return this->as_entity()->is(name);
	} else if (this->as_type_declaration()) {
		const IfcParse::named_type* nt = this->as_type_declaration()->declared_type()->as_named_type();
		if (nt) return nt->is(name);
	}

	return false;
}

bool IfcParse::declaration::is(const IfcParse::declaration& decl) const {
	if (this == &decl) return true;

	if (this->as_entity()) {
		return this->as_entity()->is(decl);
	} else if (this->as_type_declaration()) {
		const IfcParse::named_type* nt = this->as_type_declaration()->declared_type()->as_named_type();
		if (nt) return nt->is(decl);
	}

	return false;
}

bool IfcParse::named_type::is(const std::string& name) const {
	return declared_type()->is(name);
}

bool IfcParse::named_type::is(const IfcParse::declaration& decl) const {
	return declared_type()->is(decl);
}

static std::map<std::string, const IfcParse::schema_definition*> schemas;

IfcParse::schema_definition::schema_definition(const std::string& name, const std::vector<const declaration*>& declarations, instance_factory* factory)
	: name_(name)
	, declarations_(declarations)
	, factory_(factory)
{
	std::sort(declarations_.begin(), declarations_.end(), declaration_by_index_sort());
	for (std::vector<const declaration*>::iterator it = declarations_.begin(); it != declarations_.end(); ++it) {
		(**it).schema_ = this;

		if ((**it).as_type_declaration()) type_declarations_.push_back((**it).as_type_declaration());
		if ((**it).as_select_type()) select_types_.push_back((**it).as_select_type());
		if ((**it).as_enumeration_type()) enumeration_types_.push_back((**it).as_enumeration_type());
		if ((**it).as_entity()) entities_.push_back((**it).as_entity());
	}
	schemas[name_] = this;
}

IfcParse::schema_definition::~schema_definition() {
	schemas.erase(name_);
	for (std::vector<const declaration*>::const_iterator it = declarations_.begin(); it != declarations_.end(); ++it) {
		delete *it;
	}
}

IfcUtil::IfcBaseClass* IfcParse::schema_definition::instantiate(IfcEntityInstanceData * data) const {
	if (factory_) {
		return (*factory_)(data);
	} else {
		return new IfcUtil::IfcLateBoundEntity(data->type(), data);
	}
}

void IfcParse::register_schema(schema_definition* s) {
	schemas.insert({ boost::to_upper_copy(s->name()), s });
}


#ifdef HAS_SCHEMA_2x3
#include "../ifcparse/Ifc2x3.h"
#endif
#ifdef HAS_SCHEMA_4
#include "../ifcparse/Ifc4.h"
#endif
#ifdef HAS_SCHEMA_4x1
#include "../ifcparse/Ifc4x1.h"
#endif
#ifdef HAS_SCHEMA_4x2
#include "../ifcparse/Ifc4x2.h"
#endif
#ifdef HAS_SCHEMA_4x3_rc1
#include "../ifcparse/Ifc4x3_rc1.h"
#endif
#ifdef HAS_SCHEMA_4x3_rc2
#include "../ifcparse/Ifc4x3_rc2.h"
#endif
#ifdef HAS_SCHEMA_4x3_rc3
#include "../ifcparse/Ifc4x3_rc3.h"
#endif
#ifdef HAS_SCHEMA_4x3_rc4
#include "../ifcparse/Ifc4x3_rc4.h"
#endif

const IfcParse::schema_definition* IfcParse::schema_by_name(const std::string& name) {
	// TODO: initialize automatically somehow
#ifdef HAS_SCHEMA_2x3
	Ifc2x3::get_schema();
#endif
#ifdef HAS_SCHEMA_4
	Ifc4::get_schema();
#endif
#ifdef HAS_SCHEMA_4x1
	Ifc4x1::get_schema();
#endif
#ifdef HAS_SCHEMA_4x2
	Ifc4x2::get_schema();
#endif
#ifdef HAS_SCHEMA_4x3_rc1
	Ifc4x3_rc1::get_schema();
#endif
#ifdef HAS_SCHEMA_4x3_rc2
	Ifc4x3_rc2::get_schema();
#endif
#ifdef HAS_SCHEMA_4x3_rc3
   Ifc4x3_rc3::get_schema();
#endif
#ifdef HAS_SCHEMA_4x3_rc4
   Ifc4x3_rc4::get_schema();
#endif

	std::map<std::string, const IfcParse::schema_definition*>::const_iterator it = schemas.find(boost::to_upper_copy(name));
	if (it == schemas.end()) {
		throw IfcParse::IfcException("No schema named " + name);
	}
	return it->second;
}

std::vector<std::string> IfcParse::schema_names() {
	// Load schema modules
	try {
		IfcParse::schema_by_name("IFC2X3");
	} catch (IfcParse::IfcException&) {}

	// Populate vector with map keys
	std::vector<std::string> return_value;
	for (auto& pair : schemas) {
		return_value.push_back(pair.first);
	}

	return return_value;
}
