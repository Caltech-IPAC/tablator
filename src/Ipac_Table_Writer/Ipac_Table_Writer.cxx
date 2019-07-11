#include "../Ipac_Table_Writer.hxx"

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"

/**********************************************************/
/* public class member functions */
/**********************************************************/


void tablator::Ipac_Table_Writer::write_ipac_table(const tablator::Table &table,
                                                   std::ostream &os) {
    write_ipac_table(table, os,
                     Data_Type_Adjuster(table).get_datatypes_for_writing(
                             Format::Enums::IPAC_TABLE));
}

// more to come...
