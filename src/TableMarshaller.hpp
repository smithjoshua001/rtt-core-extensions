/***************************************************************************
  tag: Peter Soetens  Mon Jan 19 14:11:20 CET 2004  TableMarshaller.hpp

                        TableMarshaller.hpp -  description
                           -------------------
    begin                : Mon January 19 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef COGIMON_PROPERTIES_TABLESERIALIZER
#define COGIMON_PROPERTIES_TABLESERIALIZER

#include <rtt/Property.hpp>
#include <rtt/base/PropertyIntrospection.hpp>
#include <rtt/marsh/StreamProcessor.hpp>
#include <rtt/marsh/MarshallInterface.hpp>
#include <boost/lexical_cast.hpp>

#include <rst-rt/monitoring/CallTraceSample.hpp>

namespace RTT
{

    /**
     * A marsh::MarshallInterface for generating a stream of numbers, ordered in
     * columns. A new row is created on each flush() command. The
     * TableHeaderMarshaller can create the appropriate heading for
     * the columns.
     */
    template<typename o_stream>
    class TableMarshaller
        : public marsh::MarshallInterface, public marsh::StreamProcessor<o_stream>
    {
        std::string msep;
        bool first_time;
        public:
        typedef o_stream output_stream;
        typedef o_stream OutputStream;

        /**
         * Create a new marshaller, streaming the data to a stream.
         * @param os The stream to write the data to (i.e. cerr)
         * @param sep The separater to place between each column and at
         * the end of the line.
         */
        TableMarshaller(output_stream &os, std::string sep=",") :
            marsh::StreamProcessor<o_stream>(os), msep(sep) {
                first_time = true;
            }

            virtual ~TableMarshaller() {}

			virtual void serialize(base::PropertyBase* v) {
                Property<PropertyBag>* bag = dynamic_cast< Property<PropertyBag>* >( v );
                if (bag) {
                    this->serialize( bag->value() );
                } else {
                    if (!v->getType().compare("rstrt.monitoring.CallTraceSample[]")) {
                        boost::intrusive_ptr<RTT::base::DataSourceBase> s = v->getDataSource();
                        if (s) {
                            RTT::internal::DataSource<int>::shared_ptr size = RTT::internal::DataSource<int>::narrow(s->getMember("size").get());
                            if (size) {
                                int msize = size->get();
                                for (int i = 0; i < msize; i++) {
                                    std::string indx = boost::lexical_cast<std::string>( i );
                                    RTT::base::DataSourceBase::shared_ptr item = s->getMember(indx);
                                    // resized = new CheckSizeDataSource( msize, size, resized );
                                    if (item) {
                                        if ( !item->isAssignable() ) {
                                            // For example: the case for size() and capacity() in SequenceTypeInfo
                                            log(Warning)<<"memberDecomposition: Item '"<< indx << "' of type "<< s->getTypeName() << " is not changeable."<<endlog();
                                            continue;
                                        }
                                        
                                        // finally recurse or add it to the target bag:
                                        base::PropertyBase* newpb = item->getTypeInfo()->buildProperty(indx, "", item);

                                        if (first_time) {
                                            *this->s << newpb->getDataSource();
                                            first_time = false;
                                        } else {
                                            *this->s << msep << newpb->getDataSource();
                                        }
                                        delete newpb;
                                    }
                                }
                            }
                        }
                    } else {
                        *this->s << v->getDataSource();// NEVER DONE!
                    }
                }
			}

            virtual void serialize(const PropertyBag &v) {
                for (PropertyBag::const_iterator i = v.getProperties().begin(); i != v.getProperties().end(); i++) {
                    this->serialize( *i );
                }
			}

            virtual void flush() {
                // *this->s << std::endl;
            }
	};
}
#endif
