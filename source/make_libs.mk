.PHONY: all
all: $(BUILDIR)LibTpBoost.a $(BUILDIR)LibJpegIjg6b.a $(BUILDIR)LibFgBase.a 
FLAGSLibTpBoost =  -w -DBOOST_ALL_NO_LIB -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE -isystem LibTpBoost/boost_1_67_0/
SDIRLibTpBoost = LibTpBoost/boost_1_67_0/
ODIRLibTpBoost = $(BUILDIR)LibTpBoost/
$(shell mkdir -p $(ODIRLibTpBoost))
INCSLibTpBoost := $(wildcard LibTpBoost/boost_1_67_0/boost/*.hpp) 
$(BUILDIR)LibTpBoost.a: $(ODIRLibTpBoost)libs_filesystem_src_codecvt_error_category.o $(ODIRLibTpBoost)libs_filesystem_src_operations.o $(ODIRLibTpBoost)libs_filesystem_src_path.o $(ODIRLibTpBoost)libs_filesystem_src_path_traits.o $(ODIRLibTpBoost)libs_filesystem_src_portability.o $(ODIRLibTpBoost)libs_filesystem_src_unique_path.o $(ODIRLibTpBoost)libs_filesystem_src_utf8_codecvt_facet.o $(ODIRLibTpBoost)libs_filesystem_src_windows_file_codecvt.o $(ODIRLibTpBoost)libs_serialization_src_archive_exception.o $(ODIRLibTpBoost)libs_serialization_src_basic_archive.o $(ODIRLibTpBoost)libs_serialization_src_basic_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_basic_iserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_basic_oserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_iserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_oserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_serializer_map.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_iprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_oprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_wiprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_woprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_xml_archive.o $(ODIRLibTpBoost)libs_serialization_src_binary_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_woarchive.o $(ODIRLibTpBoost)libs_serialization_src_codecvt_null.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_no_rtti.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_typeid.o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_stl_port.o $(ODIRLibTpBoost)libs_serialization_src_text_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_woarchive.o $(ODIRLibTpBoost)libs_serialization_src_utf8_codecvt_facet2.o $(ODIRLibTpBoost)libs_serialization_src_void_cast.o $(ODIRLibTpBoost)libs_serialization_src_xml_archive_exception.o $(ODIRLibTpBoost)libs_serialization_src_xml_grammar.o $(ODIRLibTpBoost)libs_serialization_src_xml_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_wgrammar.o $(ODIRLibTpBoost)libs_serialization_src_xml_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_woarchive.o $(ODIRLibTpBoost)libs_system_src_error_code.o 
	$(AR) rc $(BUILDIR)LibTpBoost.a $(ODIRLibTpBoost)libs_filesystem_src_codecvt_error_category.o $(ODIRLibTpBoost)libs_filesystem_src_operations.o $(ODIRLibTpBoost)libs_filesystem_src_path.o $(ODIRLibTpBoost)libs_filesystem_src_path_traits.o $(ODIRLibTpBoost)libs_filesystem_src_portability.o $(ODIRLibTpBoost)libs_filesystem_src_unique_path.o $(ODIRLibTpBoost)libs_filesystem_src_utf8_codecvt_facet.o $(ODIRLibTpBoost)libs_filesystem_src_windows_file_codecvt.o $(ODIRLibTpBoost)libs_serialization_src_archive_exception.o $(ODIRLibTpBoost)libs_serialization_src_basic_archive.o $(ODIRLibTpBoost)libs_serialization_src_basic_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_basic_iserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_basic_oserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_iserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_oserializer.o $(ODIRLibTpBoost)libs_serialization_src_basic_serializer_map.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_iprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_oprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_wiprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_text_woprimitive.o $(ODIRLibTpBoost)libs_serialization_src_basic_xml_archive.o $(ODIRLibTpBoost)libs_serialization_src_binary_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_binary_woarchive.o $(ODIRLibTpBoost)libs_serialization_src_codecvt_null.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_no_rtti.o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_typeid.o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_stl_port.o $(ODIRLibTpBoost)libs_serialization_src_text_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_text_woarchive.o $(ODIRLibTpBoost)libs_serialization_src_utf8_codecvt_facet2.o $(ODIRLibTpBoost)libs_serialization_src_void_cast.o $(ODIRLibTpBoost)libs_serialization_src_xml_archive_exception.o $(ODIRLibTpBoost)libs_serialization_src_xml_grammar.o $(ODIRLibTpBoost)libs_serialization_src_xml_iarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_oarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_wgrammar.o $(ODIRLibTpBoost)libs_serialization_src_xml_wiarchive.o $(ODIRLibTpBoost)libs_serialization_src_xml_woarchive.o $(ODIRLibTpBoost)libs_system_src_error_code.o 
	$(RANLIB) $(BUILDIR)LibTpBoost.a
$(ODIRLibTpBoost)libs_filesystem_src_codecvt_error_category.o: $(SDIRLibTpBoost)libs/filesystem/src/codecvt_error_category.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_codecvt_error_category.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/codecvt_error_category.cpp
$(ODIRLibTpBoost)libs_filesystem_src_operations.o: $(SDIRLibTpBoost)libs/filesystem/src/operations.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_operations.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/operations.cpp
$(ODIRLibTpBoost)libs_filesystem_src_path.o: $(SDIRLibTpBoost)libs/filesystem/src/path.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_path.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/path.cpp
$(ODIRLibTpBoost)libs_filesystem_src_path_traits.o: $(SDIRLibTpBoost)libs/filesystem/src/path_traits.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_path_traits.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/path_traits.cpp
$(ODIRLibTpBoost)libs_filesystem_src_portability.o: $(SDIRLibTpBoost)libs/filesystem/src/portability.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_portability.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/portability.cpp
$(ODIRLibTpBoost)libs_filesystem_src_unique_path.o: $(SDIRLibTpBoost)libs/filesystem/src/unique_path.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_unique_path.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/unique_path.cpp
$(ODIRLibTpBoost)libs_filesystem_src_utf8_codecvt_facet.o: $(SDIRLibTpBoost)libs/filesystem/src/utf8_codecvt_facet.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_utf8_codecvt_facet.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/utf8_codecvt_facet.cpp
$(ODIRLibTpBoost)libs_filesystem_src_windows_file_codecvt.o: $(SDIRLibTpBoost)libs/filesystem/src/windows_file_codecvt.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_filesystem_src_windows_file_codecvt.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/filesystem/src/windows_file_codecvt.cpp
$(ODIRLibTpBoost)libs_serialization_src_archive_exception.o: $(SDIRLibTpBoost)libs/serialization/src/archive_exception.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_archive_exception.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/archive_exception.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_archive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_archive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_archive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_archive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_iarchive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_iarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_iarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_iarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_iserializer.o: $(SDIRLibTpBoost)libs/serialization/src/basic_iserializer.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_iserializer.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_iserializer.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_oarchive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_oarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_oarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_oarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_oserializer.o: $(SDIRLibTpBoost)libs/serialization/src/basic_oserializer.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_oserializer.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_oserializer.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_pointer_iserializer.o: $(SDIRLibTpBoost)libs/serialization/src/basic_pointer_iserializer.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_iserializer.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_pointer_iserializer.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_pointer_oserializer.o: $(SDIRLibTpBoost)libs/serialization/src/basic_pointer_oserializer.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_pointer_oserializer.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_pointer_oserializer.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_serializer_map.o: $(SDIRLibTpBoost)libs/serialization/src/basic_serializer_map.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_serializer_map.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_serializer_map.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_text_iprimitive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_text_iprimitive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_text_iprimitive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_text_iprimitive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_text_oprimitive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_text_oprimitive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_text_oprimitive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_text_oprimitive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_text_wiprimitive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_text_wiprimitive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_text_wiprimitive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_text_wiprimitive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_text_woprimitive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_text_woprimitive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_text_woprimitive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_text_woprimitive.cpp
$(ODIRLibTpBoost)libs_serialization_src_basic_xml_archive.o: $(SDIRLibTpBoost)libs/serialization/src/basic_xml_archive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_basic_xml_archive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/basic_xml_archive.cpp
$(ODIRLibTpBoost)libs_serialization_src_binary_iarchive.o: $(SDIRLibTpBoost)libs/serialization/src/binary_iarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_binary_iarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/binary_iarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_binary_oarchive.o: $(SDIRLibTpBoost)libs/serialization/src/binary_oarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_binary_oarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/binary_oarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_binary_wiarchive.o: $(SDIRLibTpBoost)libs/serialization/src/binary_wiarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_binary_wiarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/binary_wiarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_binary_woarchive.o: $(SDIRLibTpBoost)libs/serialization/src/binary_woarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_binary_woarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/binary_woarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_codecvt_null.o: $(SDIRLibTpBoost)libs/serialization/src/codecvt_null.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_codecvt_null.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/codecvt_null.cpp
$(ODIRLibTpBoost)libs_serialization_src_extended_type_info.o: $(SDIRLibTpBoost)libs/serialization/src/extended_type_info.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/extended_type_info.cpp
$(ODIRLibTpBoost)libs_serialization_src_extended_type_info_no_rtti.o: $(SDIRLibTpBoost)libs/serialization/src/extended_type_info_no_rtti.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_no_rtti.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/extended_type_info_no_rtti.cpp
$(ODIRLibTpBoost)libs_serialization_src_extended_type_info_typeid.o: $(SDIRLibTpBoost)libs/serialization/src/extended_type_info_typeid.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_extended_type_info_typeid.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/extended_type_info_typeid.cpp
$(ODIRLibTpBoost)libs_serialization_src_polymorphic_iarchive.o: $(SDIRLibTpBoost)libs/serialization/src/polymorphic_iarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_iarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/polymorphic_iarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_polymorphic_oarchive.o: $(SDIRLibTpBoost)libs/serialization/src/polymorphic_oarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_polymorphic_oarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/polymorphic_oarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_stl_port.o: $(SDIRLibTpBoost)libs/serialization/src/stl_port.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_stl_port.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/stl_port.cpp
$(ODIRLibTpBoost)libs_serialization_src_text_iarchive.o: $(SDIRLibTpBoost)libs/serialization/src/text_iarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_text_iarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/text_iarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_text_oarchive.o: $(SDIRLibTpBoost)libs/serialization/src/text_oarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_text_oarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/text_oarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_text_wiarchive.o: $(SDIRLibTpBoost)libs/serialization/src/text_wiarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_text_wiarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/text_wiarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_text_woarchive.o: $(SDIRLibTpBoost)libs/serialization/src/text_woarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_text_woarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/text_woarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_utf8_codecvt_facet2.o: $(SDIRLibTpBoost)libs/serialization/src/utf8_codecvt_facet2.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_utf8_codecvt_facet2.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/utf8_codecvt_facet2.cpp
$(ODIRLibTpBoost)libs_serialization_src_void_cast.o: $(SDIRLibTpBoost)libs/serialization/src/void_cast.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_void_cast.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/void_cast.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_archive_exception.o: $(SDIRLibTpBoost)libs/serialization/src/xml_archive_exception.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_archive_exception.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_archive_exception.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_grammar.o: $(SDIRLibTpBoost)libs/serialization/src/xml_grammar.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_grammar.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_grammar.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_iarchive.o: $(SDIRLibTpBoost)libs/serialization/src/xml_iarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_iarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_iarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_oarchive.o: $(SDIRLibTpBoost)libs/serialization/src/xml_oarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_oarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_oarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_wgrammar.o: $(SDIRLibTpBoost)libs/serialization/src/xml_wgrammar.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_wgrammar.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_wgrammar.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_wiarchive.o: $(SDIRLibTpBoost)libs/serialization/src/xml_wiarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_wiarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_wiarchive.cpp
$(ODIRLibTpBoost)libs_serialization_src_xml_woarchive.o: $(SDIRLibTpBoost)libs/serialization/src/xml_woarchive.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_serialization_src_xml_woarchive.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/serialization/src/xml_woarchive.cpp
$(ODIRLibTpBoost)libs_system_src_error_code.o: $(SDIRLibTpBoost)libs/system/src/error_code.cpp $(INCSLibTpBoost)
	$(CXX) -o $(ODIRLibTpBoost)libs_system_src_error_code.o -c $(CXXFLAGS) $(FLAGSLibTpBoost) $(SDIRLibTpBoost)libs/system/src/error_code.cpp
FLAGSLibJpegIjg6b =  -w -isystem LibJpegIjg6b/
SDIRLibJpegIjg6b = LibJpegIjg6b/
ODIRLibJpegIjg6b = $(BUILDIR)LibJpegIjg6b/
$(shell mkdir -p $(ODIRLibJpegIjg6b))
INCSLibJpegIjg6b := $(wildcard LibJpegIjg6b/*.hpp) 
$(BUILDIR)LibJpegIjg6b.a: $(ODIRLibJpegIjg6b)jcapimin.o $(ODIRLibJpegIjg6b)jcapistd.o $(ODIRLibJpegIjg6b)jccoefct.o $(ODIRLibJpegIjg6b)jccolor.o $(ODIRLibJpegIjg6b)jcdctmgr.o $(ODIRLibJpegIjg6b)jchuff.o $(ODIRLibJpegIjg6b)jcinit.o $(ODIRLibJpegIjg6b)jcmainct.o $(ODIRLibJpegIjg6b)jcmarker.o $(ODIRLibJpegIjg6b)jcmaster.o $(ODIRLibJpegIjg6b)jcomapi.o $(ODIRLibJpegIjg6b)jcparam.o $(ODIRLibJpegIjg6b)jcphuff.o $(ODIRLibJpegIjg6b)jcprepct.o $(ODIRLibJpegIjg6b)jcsample.o $(ODIRLibJpegIjg6b)jctrans.o $(ODIRLibJpegIjg6b)jdapimin.o $(ODIRLibJpegIjg6b)jdapistd.o $(ODIRLibJpegIjg6b)jdatadst.o $(ODIRLibJpegIjg6b)jdatasrc.o $(ODIRLibJpegIjg6b)jdcoefct.o $(ODIRLibJpegIjg6b)jdcolor.o $(ODIRLibJpegIjg6b)jddctmgr.o $(ODIRLibJpegIjg6b)jdhuff.o $(ODIRLibJpegIjg6b)jdinput.o $(ODIRLibJpegIjg6b)jdmainct.o $(ODIRLibJpegIjg6b)jdmarker.o $(ODIRLibJpegIjg6b)jdmaster.o $(ODIRLibJpegIjg6b)jdmerge.o $(ODIRLibJpegIjg6b)jdphuff.o $(ODIRLibJpegIjg6b)jdpostct.o $(ODIRLibJpegIjg6b)jdsample.o $(ODIRLibJpegIjg6b)jdtrans.o $(ODIRLibJpegIjg6b)jerror.o $(ODIRLibJpegIjg6b)jfdctflt.o $(ODIRLibJpegIjg6b)jfdctfst.o $(ODIRLibJpegIjg6b)jfdctint.o $(ODIRLibJpegIjg6b)jidctflt.o $(ODIRLibJpegIjg6b)jidctfst.o $(ODIRLibJpegIjg6b)jidctint.o $(ODIRLibJpegIjg6b)jidctred.o $(ODIRLibJpegIjg6b)jmemmgr.o $(ODIRLibJpegIjg6b)jmemnobs.o $(ODIRLibJpegIjg6b)jquant1.o $(ODIRLibJpegIjg6b)jquant2.o $(ODIRLibJpegIjg6b)jutils.o $(ODIRLibJpegIjg6b)rdbmp.o $(ODIRLibJpegIjg6b)rdcolmap.o $(ODIRLibJpegIjg6b)rdgif.o $(ODIRLibJpegIjg6b)rdppm.o $(ODIRLibJpegIjg6b)rdrle.o $(ODIRLibJpegIjg6b)rdswitch.o $(ODIRLibJpegIjg6b)rdtarga.o $(ODIRLibJpegIjg6b)transupp.o $(ODIRLibJpegIjg6b)wrbmp.o $(ODIRLibJpegIjg6b)wrgif.o $(ODIRLibJpegIjg6b)wrppm.o $(ODIRLibJpegIjg6b)wrrle.o $(ODIRLibJpegIjg6b)wrtarga.o 
	$(AR) rc $(BUILDIR)LibJpegIjg6b.a $(ODIRLibJpegIjg6b)jcapimin.o $(ODIRLibJpegIjg6b)jcapistd.o $(ODIRLibJpegIjg6b)jccoefct.o $(ODIRLibJpegIjg6b)jccolor.o $(ODIRLibJpegIjg6b)jcdctmgr.o $(ODIRLibJpegIjg6b)jchuff.o $(ODIRLibJpegIjg6b)jcinit.o $(ODIRLibJpegIjg6b)jcmainct.o $(ODIRLibJpegIjg6b)jcmarker.o $(ODIRLibJpegIjg6b)jcmaster.o $(ODIRLibJpegIjg6b)jcomapi.o $(ODIRLibJpegIjg6b)jcparam.o $(ODIRLibJpegIjg6b)jcphuff.o $(ODIRLibJpegIjg6b)jcprepct.o $(ODIRLibJpegIjg6b)jcsample.o $(ODIRLibJpegIjg6b)jctrans.o $(ODIRLibJpegIjg6b)jdapimin.o $(ODIRLibJpegIjg6b)jdapistd.o $(ODIRLibJpegIjg6b)jdatadst.o $(ODIRLibJpegIjg6b)jdatasrc.o $(ODIRLibJpegIjg6b)jdcoefct.o $(ODIRLibJpegIjg6b)jdcolor.o $(ODIRLibJpegIjg6b)jddctmgr.o $(ODIRLibJpegIjg6b)jdhuff.o $(ODIRLibJpegIjg6b)jdinput.o $(ODIRLibJpegIjg6b)jdmainct.o $(ODIRLibJpegIjg6b)jdmarker.o $(ODIRLibJpegIjg6b)jdmaster.o $(ODIRLibJpegIjg6b)jdmerge.o $(ODIRLibJpegIjg6b)jdphuff.o $(ODIRLibJpegIjg6b)jdpostct.o $(ODIRLibJpegIjg6b)jdsample.o $(ODIRLibJpegIjg6b)jdtrans.o $(ODIRLibJpegIjg6b)jerror.o $(ODIRLibJpegIjg6b)jfdctflt.o $(ODIRLibJpegIjg6b)jfdctfst.o $(ODIRLibJpegIjg6b)jfdctint.o $(ODIRLibJpegIjg6b)jidctflt.o $(ODIRLibJpegIjg6b)jidctfst.o $(ODIRLibJpegIjg6b)jidctint.o $(ODIRLibJpegIjg6b)jidctred.o $(ODIRLibJpegIjg6b)jmemmgr.o $(ODIRLibJpegIjg6b)jmemnobs.o $(ODIRLibJpegIjg6b)jquant1.o $(ODIRLibJpegIjg6b)jquant2.o $(ODIRLibJpegIjg6b)jutils.o $(ODIRLibJpegIjg6b)rdbmp.o $(ODIRLibJpegIjg6b)rdcolmap.o $(ODIRLibJpegIjg6b)rdgif.o $(ODIRLibJpegIjg6b)rdppm.o $(ODIRLibJpegIjg6b)rdrle.o $(ODIRLibJpegIjg6b)rdswitch.o $(ODIRLibJpegIjg6b)rdtarga.o $(ODIRLibJpegIjg6b)transupp.o $(ODIRLibJpegIjg6b)wrbmp.o $(ODIRLibJpegIjg6b)wrgif.o $(ODIRLibJpegIjg6b)wrppm.o $(ODIRLibJpegIjg6b)wrrle.o $(ODIRLibJpegIjg6b)wrtarga.o 
	$(RANLIB) $(BUILDIR)LibJpegIjg6b.a
$(ODIRLibJpegIjg6b)jcapimin.o: $(SDIRLibJpegIjg6b)jcapimin.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcapimin.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcapimin.c
$(ODIRLibJpegIjg6b)jcapistd.o: $(SDIRLibJpegIjg6b)jcapistd.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcapistd.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcapistd.c
$(ODIRLibJpegIjg6b)jccoefct.o: $(SDIRLibJpegIjg6b)jccoefct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jccoefct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jccoefct.c
$(ODIRLibJpegIjg6b)jccolor.o: $(SDIRLibJpegIjg6b)jccolor.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jccolor.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jccolor.c
$(ODIRLibJpegIjg6b)jcdctmgr.o: $(SDIRLibJpegIjg6b)jcdctmgr.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcdctmgr.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcdctmgr.c
$(ODIRLibJpegIjg6b)jchuff.o: $(SDIRLibJpegIjg6b)jchuff.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jchuff.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jchuff.c
$(ODIRLibJpegIjg6b)jcinit.o: $(SDIRLibJpegIjg6b)jcinit.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcinit.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcinit.c
$(ODIRLibJpegIjg6b)jcmainct.o: $(SDIRLibJpegIjg6b)jcmainct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcmainct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcmainct.c
$(ODIRLibJpegIjg6b)jcmarker.o: $(SDIRLibJpegIjg6b)jcmarker.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcmarker.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcmarker.c
$(ODIRLibJpegIjg6b)jcmaster.o: $(SDIRLibJpegIjg6b)jcmaster.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcmaster.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcmaster.c
$(ODIRLibJpegIjg6b)jcomapi.o: $(SDIRLibJpegIjg6b)jcomapi.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcomapi.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcomapi.c
$(ODIRLibJpegIjg6b)jcparam.o: $(SDIRLibJpegIjg6b)jcparam.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcparam.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcparam.c
$(ODIRLibJpegIjg6b)jcphuff.o: $(SDIRLibJpegIjg6b)jcphuff.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcphuff.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcphuff.c
$(ODIRLibJpegIjg6b)jcprepct.o: $(SDIRLibJpegIjg6b)jcprepct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcprepct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcprepct.c
$(ODIRLibJpegIjg6b)jcsample.o: $(SDIRLibJpegIjg6b)jcsample.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jcsample.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jcsample.c
$(ODIRLibJpegIjg6b)jctrans.o: $(SDIRLibJpegIjg6b)jctrans.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jctrans.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jctrans.c
$(ODIRLibJpegIjg6b)jdapimin.o: $(SDIRLibJpegIjg6b)jdapimin.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdapimin.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdapimin.c
$(ODIRLibJpegIjg6b)jdapistd.o: $(SDIRLibJpegIjg6b)jdapistd.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdapistd.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdapistd.c
$(ODIRLibJpegIjg6b)jdatadst.o: $(SDIRLibJpegIjg6b)jdatadst.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdatadst.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdatadst.c
$(ODIRLibJpegIjg6b)jdatasrc.o: $(SDIRLibJpegIjg6b)jdatasrc.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdatasrc.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdatasrc.c
$(ODIRLibJpegIjg6b)jdcoefct.o: $(SDIRLibJpegIjg6b)jdcoefct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdcoefct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdcoefct.c
$(ODIRLibJpegIjg6b)jdcolor.o: $(SDIRLibJpegIjg6b)jdcolor.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdcolor.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdcolor.c
$(ODIRLibJpegIjg6b)jddctmgr.o: $(SDIRLibJpegIjg6b)jddctmgr.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jddctmgr.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jddctmgr.c
$(ODIRLibJpegIjg6b)jdhuff.o: $(SDIRLibJpegIjg6b)jdhuff.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdhuff.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdhuff.c
$(ODIRLibJpegIjg6b)jdinput.o: $(SDIRLibJpegIjg6b)jdinput.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdinput.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdinput.c
$(ODIRLibJpegIjg6b)jdmainct.o: $(SDIRLibJpegIjg6b)jdmainct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdmainct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdmainct.c
$(ODIRLibJpegIjg6b)jdmarker.o: $(SDIRLibJpegIjg6b)jdmarker.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdmarker.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdmarker.c
$(ODIRLibJpegIjg6b)jdmaster.o: $(SDIRLibJpegIjg6b)jdmaster.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdmaster.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdmaster.c
$(ODIRLibJpegIjg6b)jdmerge.o: $(SDIRLibJpegIjg6b)jdmerge.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdmerge.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdmerge.c
$(ODIRLibJpegIjg6b)jdphuff.o: $(SDIRLibJpegIjg6b)jdphuff.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdphuff.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdphuff.c
$(ODIRLibJpegIjg6b)jdpostct.o: $(SDIRLibJpegIjg6b)jdpostct.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdpostct.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdpostct.c
$(ODIRLibJpegIjg6b)jdsample.o: $(SDIRLibJpegIjg6b)jdsample.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdsample.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdsample.c
$(ODIRLibJpegIjg6b)jdtrans.o: $(SDIRLibJpegIjg6b)jdtrans.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jdtrans.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jdtrans.c
$(ODIRLibJpegIjg6b)jerror.o: $(SDIRLibJpegIjg6b)jerror.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jerror.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jerror.c
$(ODIRLibJpegIjg6b)jfdctflt.o: $(SDIRLibJpegIjg6b)jfdctflt.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jfdctflt.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jfdctflt.c
$(ODIRLibJpegIjg6b)jfdctfst.o: $(SDIRLibJpegIjg6b)jfdctfst.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jfdctfst.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jfdctfst.c
$(ODIRLibJpegIjg6b)jfdctint.o: $(SDIRLibJpegIjg6b)jfdctint.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jfdctint.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jfdctint.c
$(ODIRLibJpegIjg6b)jidctflt.o: $(SDIRLibJpegIjg6b)jidctflt.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jidctflt.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jidctflt.c
$(ODIRLibJpegIjg6b)jidctfst.o: $(SDIRLibJpegIjg6b)jidctfst.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jidctfst.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jidctfst.c
$(ODIRLibJpegIjg6b)jidctint.o: $(SDIRLibJpegIjg6b)jidctint.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jidctint.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jidctint.c
$(ODIRLibJpegIjg6b)jidctred.o: $(SDIRLibJpegIjg6b)jidctred.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jidctred.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jidctred.c
$(ODIRLibJpegIjg6b)jmemmgr.o: $(SDIRLibJpegIjg6b)jmemmgr.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jmemmgr.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jmemmgr.c
$(ODIRLibJpegIjg6b)jmemnobs.o: $(SDIRLibJpegIjg6b)jmemnobs.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jmemnobs.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jmemnobs.c
$(ODIRLibJpegIjg6b)jquant1.o: $(SDIRLibJpegIjg6b)jquant1.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jquant1.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jquant1.c
$(ODIRLibJpegIjg6b)jquant2.o: $(SDIRLibJpegIjg6b)jquant2.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jquant2.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jquant2.c
$(ODIRLibJpegIjg6b)jutils.o: $(SDIRLibJpegIjg6b)jutils.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)jutils.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)jutils.c
$(ODIRLibJpegIjg6b)rdbmp.o: $(SDIRLibJpegIjg6b)rdbmp.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdbmp.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdbmp.c
$(ODIRLibJpegIjg6b)rdcolmap.o: $(SDIRLibJpegIjg6b)rdcolmap.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdcolmap.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdcolmap.c
$(ODIRLibJpegIjg6b)rdgif.o: $(SDIRLibJpegIjg6b)rdgif.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdgif.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdgif.c
$(ODIRLibJpegIjg6b)rdppm.o: $(SDIRLibJpegIjg6b)rdppm.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdppm.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdppm.c
$(ODIRLibJpegIjg6b)rdrle.o: $(SDIRLibJpegIjg6b)rdrle.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdrle.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdrle.c
$(ODIRLibJpegIjg6b)rdswitch.o: $(SDIRLibJpegIjg6b)rdswitch.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdswitch.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdswitch.c
$(ODIRLibJpegIjg6b)rdtarga.o: $(SDIRLibJpegIjg6b)rdtarga.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)rdtarga.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)rdtarga.c
$(ODIRLibJpegIjg6b)transupp.o: $(SDIRLibJpegIjg6b)transupp.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)transupp.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)transupp.c
$(ODIRLibJpegIjg6b)wrbmp.o: $(SDIRLibJpegIjg6b)wrbmp.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)wrbmp.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)wrbmp.c
$(ODIRLibJpegIjg6b)wrgif.o: $(SDIRLibJpegIjg6b)wrgif.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)wrgif.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)wrgif.c
$(ODIRLibJpegIjg6b)wrppm.o: $(SDIRLibJpegIjg6b)wrppm.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)wrppm.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)wrppm.c
$(ODIRLibJpegIjg6b)wrrle.o: $(SDIRLibJpegIjg6b)wrrle.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)wrrle.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)wrrle.c
$(ODIRLibJpegIjg6b)wrtarga.o: $(SDIRLibJpegIjg6b)wrtarga.c $(INCSLibJpegIjg6b)
	$(CC) -o $(ODIRLibJpegIjg6b)wrtarga.o -c $(CCFLAGS) $(FLAGSLibJpegIjg6b) $(SDIRLibJpegIjg6b)wrtarga.c
FLAGSLibFgBase =  -Wall -Wextra -DBOOST_ALL_NO_LIB -DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE -ILibFgBase/src/ -isystem LibTpEigen/ -isystem LibJpegIjg6b/ -isystem LibTpStb/stb/ -isystem LibTpBoost/boost_1_67_0/
SDIRLibFgBase = LibFgBase/src/
ODIRLibFgBase = $(BUILDIR)LibFgBase/
$(shell mkdir -p $(ODIRLibFgBase))
INCSLibFgBase := $(wildcard LibFgBase/src/*.hpp) $(wildcard LibTpEigen/Eigen/*.hpp) $(wildcard LibJpegIjg6b/*.hpp) $(wildcard LibTpStb/stb/*.hpp) $(wildcard LibTpBoost/boost_1_67_0/boost/*.hpp) 
$(BUILDIR)LibFgBase.a: $(ODIRLibFgBase)Fg3dCamera.o $(ODIRLibFgBase)Fg3dDisplay.o $(ODIRLibFgBase)Fg3dMesh.o $(ODIRLibFgBase)Fg3dMesh3ds.o $(ODIRLibFgBase)Fg3dMeshDae.o $(ODIRLibFgBase)Fg3dMeshFbx.o $(ODIRLibFgBase)Fg3dMeshFgmesh.o $(ODIRLibFgBase)Fg3dMeshIo.o $(ODIRLibFgBase)Fg3dMeshLegacy.o $(ODIRLibFgBase)Fg3dMeshLwo.o $(ODIRLibFgBase)Fg3dMeshMa.o $(ODIRLibFgBase)Fg3dMeshObj.o $(ODIRLibFgBase)Fg3dMeshOps.o $(ODIRLibFgBase)Fg3dMeshPly.o $(ODIRLibFgBase)Fg3dMeshStl.o $(ODIRLibFgBase)Fg3dMeshTri.o $(ODIRLibFgBase)Fg3dMeshVrml.o $(ODIRLibFgBase)Fg3dMeshXsi.o $(ODIRLibFgBase)Fg3dSurface.o $(ODIRLibFgBase)Fg3dTest.o $(ODIRLibFgBase)FgAffine.o $(ODIRLibFgBase)FgApproxFunc.o $(ODIRLibFgBase)FgBuild.o $(ODIRLibFgBase)FgBuildMakefiles.o $(ODIRLibFgBase)FgBuildVisualStudioSln.o $(ODIRLibFgBase)FgCl.o $(ODIRLibFgBase)FgCluster.o $(ODIRLibFgBase)FgCmdBase.o $(ODIRLibFgBase)FgCmdImage.o $(ODIRLibFgBase)FgCmdMesh.o $(ODIRLibFgBase)FgCmdMorph.o $(ODIRLibFgBase)FgCmdRender.o $(ODIRLibFgBase)FgCmdTestmCpp.o $(ODIRLibFgBase)FgCmdView.o $(ODIRLibFgBase)FgCommand.o $(ODIRLibFgBase)FgDataflow.o $(ODIRLibFgBase)FgDiagnostics.o $(ODIRLibFgBase)FgException.o $(ODIRLibFgBase)FgExceptionTest.o $(ODIRLibFgBase)FgFileSystem.o $(ODIRLibFgBase)FgFileSystemTest.o $(ODIRLibFgBase)FgFileUtils.o $(ODIRLibFgBase)FgGeometry.o $(ODIRLibFgBase)FgGridIndex.o $(ODIRLibFgBase)FgGridTriangles.o $(ODIRLibFgBase)FgGuiApi.o $(ODIRLibFgBase)FgGuiApi3d.o $(ODIRLibFgBase)FgGuiApiBase.o $(ODIRLibFgBase)FgGuiApiButton.o $(ODIRLibFgBase)FgGuiApiCheckbox.o $(ODIRLibFgBase)FgGuiApiDialogs.o $(ODIRLibFgBase)FgGuiApiImage.o $(ODIRLibFgBase)FgGuiApiRadio.o $(ODIRLibFgBase)FgGuiApiSlider.o $(ODIRLibFgBase)FgGuiApiSplit.o $(ODIRLibFgBase)FgGuiApiText.o $(ODIRLibFgBase)FgHash.o $(ODIRLibFgBase)FgHex.o $(ODIRLibFgBase)FgHistogram.o $(ODIRLibFgBase)FgImage.o $(ODIRLibFgBase)FgImageDraw.o $(ODIRLibFgBase)FgImageIo.o $(ODIRLibFgBase)FgImageIoStb.o $(ODIRLibFgBase)FgImagePoint.o $(ODIRLibFgBase)FgImageTest.o $(ODIRLibFgBase)FgImgDisplay.o $(ODIRLibFgBase)FgImgJpeg.o $(ODIRLibFgBase)FgKdTree.o $(ODIRLibFgBase)FgLighting.o $(ODIRLibFgBase)FgMain.o $(ODIRLibFgBase)FgMath.o $(ODIRLibFgBase)FgMatrixC.o $(ODIRLibFgBase)FgMatrixSolver.o $(ODIRLibFgBase)FgMatrixSolverEigen.o $(ODIRLibFgBase)FgMatrixV.o $(ODIRLibFgBase)FgMetaFormat.o $(ODIRLibFgBase)FgNc.o $(ODIRLibFgBase)FgOut.o $(ODIRLibFgBase)FgParse.o $(ODIRLibFgBase)FgPath.o $(ODIRLibFgBase)FgPlatform.o $(ODIRLibFgBase)FgQuaternion.o $(ODIRLibFgBase)FgRandom.o $(ODIRLibFgBase)FgRayCaster.o $(ODIRLibFgBase)FgSampler.o $(ODIRLibFgBase)FgSerial.o $(ODIRLibFgBase)FgSimilarity.o $(ODIRLibFgBase)FgSoftRender.o $(ODIRLibFgBase)FgStdExtensions.o $(ODIRLibFgBase)FgStdio.o $(ODIRLibFgBase)FgStdStream.o $(ODIRLibFgBase)FgStdString.o $(ODIRLibFgBase)FgString.o $(ODIRLibFgBase)FgStringTest.o $(ODIRLibFgBase)FgSyntax.o $(ODIRLibFgBase)FgTcpTest.o $(ODIRLibFgBase)FgTestUtils.o $(ODIRLibFgBase)FgTime.o $(ODIRLibFgBase)FgTopology.o $(ODIRLibFgBase)jpeg_mem_dest.o $(ODIRLibFgBase)jpeg_mem_src.o $(ODIRLibFgBase)MurmurHash2.o $(ODIRLibFgBase)portable_binary_iarchive.o $(ODIRLibFgBase)portable_binary_oarchive.o $(ODIRLibFgBase)stdafx.o $(ODIRLibFgBase)nix_FgClusterNix.o $(ODIRLibFgBase)nix_FgConioNix.o $(ODIRLibFgBase)nix_FgFileSystemNix.o $(ODIRLibFgBase)nix_FgGuiNix.o $(ODIRLibFgBase)nix_FgSystemInfoNix.o $(ODIRLibFgBase)nix_FgTcpNix.o $(ODIRLibFgBase)nix_FgTimeNix.o $(ODIRLibFgBase)nix_FgWinSpecificNix.o 
	$(AR) rc $(BUILDIR)LibFgBase.a $(ODIRLibFgBase)Fg3dCamera.o $(ODIRLibFgBase)Fg3dDisplay.o $(ODIRLibFgBase)Fg3dMesh.o $(ODIRLibFgBase)Fg3dMesh3ds.o $(ODIRLibFgBase)Fg3dMeshDae.o $(ODIRLibFgBase)Fg3dMeshFbx.o $(ODIRLibFgBase)Fg3dMeshFgmesh.o $(ODIRLibFgBase)Fg3dMeshIo.o $(ODIRLibFgBase)Fg3dMeshLegacy.o $(ODIRLibFgBase)Fg3dMeshLwo.o $(ODIRLibFgBase)Fg3dMeshMa.o $(ODIRLibFgBase)Fg3dMeshObj.o $(ODIRLibFgBase)Fg3dMeshOps.o $(ODIRLibFgBase)Fg3dMeshPly.o $(ODIRLibFgBase)Fg3dMeshStl.o $(ODIRLibFgBase)Fg3dMeshTri.o $(ODIRLibFgBase)Fg3dMeshVrml.o $(ODIRLibFgBase)Fg3dMeshXsi.o $(ODIRLibFgBase)Fg3dSurface.o $(ODIRLibFgBase)Fg3dTest.o $(ODIRLibFgBase)FgAffine.o $(ODIRLibFgBase)FgApproxFunc.o $(ODIRLibFgBase)FgBuild.o $(ODIRLibFgBase)FgBuildMakefiles.o $(ODIRLibFgBase)FgBuildVisualStudioSln.o $(ODIRLibFgBase)FgCl.o $(ODIRLibFgBase)FgCluster.o $(ODIRLibFgBase)FgCmdBase.o $(ODIRLibFgBase)FgCmdImage.o $(ODIRLibFgBase)FgCmdMesh.o $(ODIRLibFgBase)FgCmdMorph.o $(ODIRLibFgBase)FgCmdRender.o $(ODIRLibFgBase)FgCmdTestmCpp.o $(ODIRLibFgBase)FgCmdView.o $(ODIRLibFgBase)FgCommand.o $(ODIRLibFgBase)FgDataflow.o $(ODIRLibFgBase)FgDiagnostics.o $(ODIRLibFgBase)FgException.o $(ODIRLibFgBase)FgExceptionTest.o $(ODIRLibFgBase)FgFileSystem.o $(ODIRLibFgBase)FgFileSystemTest.o $(ODIRLibFgBase)FgFileUtils.o $(ODIRLibFgBase)FgGeometry.o $(ODIRLibFgBase)FgGridIndex.o $(ODIRLibFgBase)FgGridTriangles.o $(ODIRLibFgBase)FgGuiApi.o $(ODIRLibFgBase)FgGuiApi3d.o $(ODIRLibFgBase)FgGuiApiBase.o $(ODIRLibFgBase)FgGuiApiButton.o $(ODIRLibFgBase)FgGuiApiCheckbox.o $(ODIRLibFgBase)FgGuiApiDialogs.o $(ODIRLibFgBase)FgGuiApiImage.o $(ODIRLibFgBase)FgGuiApiRadio.o $(ODIRLibFgBase)FgGuiApiSlider.o $(ODIRLibFgBase)FgGuiApiSplit.o $(ODIRLibFgBase)FgGuiApiText.o $(ODIRLibFgBase)FgHash.o $(ODIRLibFgBase)FgHex.o $(ODIRLibFgBase)FgHistogram.o $(ODIRLibFgBase)FgImage.o $(ODIRLibFgBase)FgImageDraw.o $(ODIRLibFgBase)FgImageIo.o $(ODIRLibFgBase)FgImageIoStb.o $(ODIRLibFgBase)FgImagePoint.o $(ODIRLibFgBase)FgImageTest.o $(ODIRLibFgBase)FgImgDisplay.o $(ODIRLibFgBase)FgImgJpeg.o $(ODIRLibFgBase)FgKdTree.o $(ODIRLibFgBase)FgLighting.o $(ODIRLibFgBase)FgMain.o $(ODIRLibFgBase)FgMath.o $(ODIRLibFgBase)FgMatrixC.o $(ODIRLibFgBase)FgMatrixSolver.o $(ODIRLibFgBase)FgMatrixSolverEigen.o $(ODIRLibFgBase)FgMatrixV.o $(ODIRLibFgBase)FgMetaFormat.o $(ODIRLibFgBase)FgNc.o $(ODIRLibFgBase)FgOut.o $(ODIRLibFgBase)FgParse.o $(ODIRLibFgBase)FgPath.o $(ODIRLibFgBase)FgPlatform.o $(ODIRLibFgBase)FgQuaternion.o $(ODIRLibFgBase)FgRandom.o $(ODIRLibFgBase)FgRayCaster.o $(ODIRLibFgBase)FgSampler.o $(ODIRLibFgBase)FgSerial.o $(ODIRLibFgBase)FgSimilarity.o $(ODIRLibFgBase)FgSoftRender.o $(ODIRLibFgBase)FgStdExtensions.o $(ODIRLibFgBase)FgStdio.o $(ODIRLibFgBase)FgStdStream.o $(ODIRLibFgBase)FgStdString.o $(ODIRLibFgBase)FgString.o $(ODIRLibFgBase)FgStringTest.o $(ODIRLibFgBase)FgSyntax.o $(ODIRLibFgBase)FgTcpTest.o $(ODIRLibFgBase)FgTestUtils.o $(ODIRLibFgBase)FgTime.o $(ODIRLibFgBase)FgTopology.o $(ODIRLibFgBase)jpeg_mem_dest.o $(ODIRLibFgBase)jpeg_mem_src.o $(ODIRLibFgBase)MurmurHash2.o $(ODIRLibFgBase)portable_binary_iarchive.o $(ODIRLibFgBase)portable_binary_oarchive.o $(ODIRLibFgBase)stdafx.o $(ODIRLibFgBase)nix_FgClusterNix.o $(ODIRLibFgBase)nix_FgConioNix.o $(ODIRLibFgBase)nix_FgFileSystemNix.o $(ODIRLibFgBase)nix_FgGuiNix.o $(ODIRLibFgBase)nix_FgSystemInfoNix.o $(ODIRLibFgBase)nix_FgTcpNix.o $(ODIRLibFgBase)nix_FgTimeNix.o $(ODIRLibFgBase)nix_FgWinSpecificNix.o 
	$(RANLIB) $(BUILDIR)LibFgBase.a
$(ODIRLibFgBase)Fg3dCamera.o: $(SDIRLibFgBase)Fg3dCamera.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dCamera.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dCamera.cpp
$(ODIRLibFgBase)Fg3dDisplay.o: $(SDIRLibFgBase)Fg3dDisplay.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dDisplay.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dDisplay.cpp
$(ODIRLibFgBase)Fg3dMesh.o: $(SDIRLibFgBase)Fg3dMesh.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMesh.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMesh.cpp
$(ODIRLibFgBase)Fg3dMesh3ds.o: $(SDIRLibFgBase)Fg3dMesh3ds.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMesh3ds.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMesh3ds.cpp
$(ODIRLibFgBase)Fg3dMeshDae.o: $(SDIRLibFgBase)Fg3dMeshDae.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshDae.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshDae.cpp
$(ODIRLibFgBase)Fg3dMeshFbx.o: $(SDIRLibFgBase)Fg3dMeshFbx.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshFbx.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshFbx.cpp
$(ODIRLibFgBase)Fg3dMeshFgmesh.o: $(SDIRLibFgBase)Fg3dMeshFgmesh.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshFgmesh.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshFgmesh.cpp
$(ODIRLibFgBase)Fg3dMeshIo.o: $(SDIRLibFgBase)Fg3dMeshIo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshIo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshIo.cpp
$(ODIRLibFgBase)Fg3dMeshLegacy.o: $(SDIRLibFgBase)Fg3dMeshLegacy.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshLegacy.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshLegacy.cpp
$(ODIRLibFgBase)Fg3dMeshLwo.o: $(SDIRLibFgBase)Fg3dMeshLwo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshLwo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshLwo.cpp
$(ODIRLibFgBase)Fg3dMeshMa.o: $(SDIRLibFgBase)Fg3dMeshMa.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshMa.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshMa.cpp
$(ODIRLibFgBase)Fg3dMeshObj.o: $(SDIRLibFgBase)Fg3dMeshObj.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshObj.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshObj.cpp
$(ODIRLibFgBase)Fg3dMeshOps.o: $(SDIRLibFgBase)Fg3dMeshOps.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshOps.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshOps.cpp
$(ODIRLibFgBase)Fg3dMeshPly.o: $(SDIRLibFgBase)Fg3dMeshPly.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshPly.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshPly.cpp
$(ODIRLibFgBase)Fg3dMeshStl.o: $(SDIRLibFgBase)Fg3dMeshStl.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshStl.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshStl.cpp
$(ODIRLibFgBase)Fg3dMeshTri.o: $(SDIRLibFgBase)Fg3dMeshTri.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshTri.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshTri.cpp
$(ODIRLibFgBase)Fg3dMeshVrml.o: $(SDIRLibFgBase)Fg3dMeshVrml.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshVrml.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshVrml.cpp
$(ODIRLibFgBase)Fg3dMeshXsi.o: $(SDIRLibFgBase)Fg3dMeshXsi.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dMeshXsi.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dMeshXsi.cpp
$(ODIRLibFgBase)Fg3dSurface.o: $(SDIRLibFgBase)Fg3dSurface.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dSurface.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dSurface.cpp
$(ODIRLibFgBase)Fg3dTest.o: $(SDIRLibFgBase)Fg3dTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)Fg3dTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)Fg3dTest.cpp
$(ODIRLibFgBase)FgAffine.o: $(SDIRLibFgBase)FgAffine.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgAffine.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgAffine.cpp
$(ODIRLibFgBase)FgApproxFunc.o: $(SDIRLibFgBase)FgApproxFunc.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgApproxFunc.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgApproxFunc.cpp
$(ODIRLibFgBase)FgBuild.o: $(SDIRLibFgBase)FgBuild.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuild.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuild.cpp
$(ODIRLibFgBase)FgBuildMakefiles.o: $(SDIRLibFgBase)FgBuildMakefiles.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuildMakefiles.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuildMakefiles.cpp
$(ODIRLibFgBase)FgBuildVisualStudioSln.o: $(SDIRLibFgBase)FgBuildVisualStudioSln.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgBuildVisualStudioSln.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgBuildVisualStudioSln.cpp
$(ODIRLibFgBase)FgCl.o: $(SDIRLibFgBase)FgCl.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCl.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCl.cpp
$(ODIRLibFgBase)FgCluster.o: $(SDIRLibFgBase)FgCluster.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCluster.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCluster.cpp
$(ODIRLibFgBase)FgCmdBase.o: $(SDIRLibFgBase)FgCmdBase.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdBase.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdBase.cpp
$(ODIRLibFgBase)FgCmdImage.o: $(SDIRLibFgBase)FgCmdImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdImage.cpp
$(ODIRLibFgBase)FgCmdMesh.o: $(SDIRLibFgBase)FgCmdMesh.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdMesh.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdMesh.cpp
$(ODIRLibFgBase)FgCmdMorph.o: $(SDIRLibFgBase)FgCmdMorph.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdMorph.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdMorph.cpp
$(ODIRLibFgBase)FgCmdRender.o: $(SDIRLibFgBase)FgCmdRender.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdRender.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdRender.cpp
$(ODIRLibFgBase)FgCmdTestmCpp.o: $(SDIRLibFgBase)FgCmdTestmCpp.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdTestmCpp.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdTestmCpp.cpp
$(ODIRLibFgBase)FgCmdView.o: $(SDIRLibFgBase)FgCmdView.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCmdView.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCmdView.cpp
$(ODIRLibFgBase)FgCommand.o: $(SDIRLibFgBase)FgCommand.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgCommand.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgCommand.cpp
$(ODIRLibFgBase)FgDataflow.o: $(SDIRLibFgBase)FgDataflow.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgDataflow.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgDataflow.cpp
$(ODIRLibFgBase)FgDiagnostics.o: $(SDIRLibFgBase)FgDiagnostics.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgDiagnostics.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgDiagnostics.cpp
$(ODIRLibFgBase)FgException.o: $(SDIRLibFgBase)FgException.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgException.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgException.cpp
$(ODIRLibFgBase)FgExceptionTest.o: $(SDIRLibFgBase)FgExceptionTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgExceptionTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgExceptionTest.cpp
$(ODIRLibFgBase)FgFileSystem.o: $(SDIRLibFgBase)FgFileSystem.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgFileSystem.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgFileSystem.cpp
$(ODIRLibFgBase)FgFileSystemTest.o: $(SDIRLibFgBase)FgFileSystemTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgFileSystemTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgFileSystemTest.cpp
$(ODIRLibFgBase)FgFileUtils.o: $(SDIRLibFgBase)FgFileUtils.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgFileUtils.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgFileUtils.cpp
$(ODIRLibFgBase)FgGeometry.o: $(SDIRLibFgBase)FgGeometry.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGeometry.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGeometry.cpp
$(ODIRLibFgBase)FgGridIndex.o: $(SDIRLibFgBase)FgGridIndex.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGridIndex.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGridIndex.cpp
$(ODIRLibFgBase)FgGridTriangles.o: $(SDIRLibFgBase)FgGridTriangles.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGridTriangles.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGridTriangles.cpp
$(ODIRLibFgBase)FgGuiApi.o: $(SDIRLibFgBase)FgGuiApi.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApi.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApi.cpp
$(ODIRLibFgBase)FgGuiApi3d.o: $(SDIRLibFgBase)FgGuiApi3d.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApi3d.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApi3d.cpp
$(ODIRLibFgBase)FgGuiApiBase.o: $(SDIRLibFgBase)FgGuiApiBase.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiBase.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiBase.cpp
$(ODIRLibFgBase)FgGuiApiButton.o: $(SDIRLibFgBase)FgGuiApiButton.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiButton.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiButton.cpp
$(ODIRLibFgBase)FgGuiApiCheckbox.o: $(SDIRLibFgBase)FgGuiApiCheckbox.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiCheckbox.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiCheckbox.cpp
$(ODIRLibFgBase)FgGuiApiDialogs.o: $(SDIRLibFgBase)FgGuiApiDialogs.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiDialogs.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiDialogs.cpp
$(ODIRLibFgBase)FgGuiApiImage.o: $(SDIRLibFgBase)FgGuiApiImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiImage.cpp
$(ODIRLibFgBase)FgGuiApiRadio.o: $(SDIRLibFgBase)FgGuiApiRadio.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiRadio.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiRadio.cpp
$(ODIRLibFgBase)FgGuiApiSlider.o: $(SDIRLibFgBase)FgGuiApiSlider.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiSlider.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiSlider.cpp
$(ODIRLibFgBase)FgGuiApiSplit.o: $(SDIRLibFgBase)FgGuiApiSplit.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiSplit.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiSplit.cpp
$(ODIRLibFgBase)FgGuiApiText.o: $(SDIRLibFgBase)FgGuiApiText.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgGuiApiText.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgGuiApiText.cpp
$(ODIRLibFgBase)FgHash.o: $(SDIRLibFgBase)FgHash.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgHash.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgHash.cpp
$(ODIRLibFgBase)FgHex.o: $(SDIRLibFgBase)FgHex.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgHex.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgHex.cpp
$(ODIRLibFgBase)FgHistogram.o: $(SDIRLibFgBase)FgHistogram.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgHistogram.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgHistogram.cpp
$(ODIRLibFgBase)FgImage.o: $(SDIRLibFgBase)FgImage.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImage.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImage.cpp
$(ODIRLibFgBase)FgImageDraw.o: $(SDIRLibFgBase)FgImageDraw.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageDraw.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageDraw.cpp
$(ODIRLibFgBase)FgImageIo.o: $(SDIRLibFgBase)FgImageIo.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageIo.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageIo.cpp
$(ODIRLibFgBase)FgImageIoStb.o: $(SDIRLibFgBase)FgImageIoStb.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageIoStb.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageIoStb.cpp
$(ODIRLibFgBase)FgImagePoint.o: $(SDIRLibFgBase)FgImagePoint.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImagePoint.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImagePoint.cpp
$(ODIRLibFgBase)FgImageTest.o: $(SDIRLibFgBase)FgImageTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImageTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImageTest.cpp
$(ODIRLibFgBase)FgImgDisplay.o: $(SDIRLibFgBase)FgImgDisplay.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImgDisplay.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImgDisplay.cpp
$(ODIRLibFgBase)FgImgJpeg.o: $(SDIRLibFgBase)FgImgJpeg.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgImgJpeg.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgImgJpeg.cpp
$(ODIRLibFgBase)FgKdTree.o: $(SDIRLibFgBase)FgKdTree.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgKdTree.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgKdTree.cpp
$(ODIRLibFgBase)FgLighting.o: $(SDIRLibFgBase)FgLighting.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgLighting.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgLighting.cpp
$(ODIRLibFgBase)FgMain.o: $(SDIRLibFgBase)FgMain.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMain.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMain.cpp
$(ODIRLibFgBase)FgMath.o: $(SDIRLibFgBase)FgMath.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMath.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMath.cpp
$(ODIRLibFgBase)FgMatrixC.o: $(SDIRLibFgBase)FgMatrixC.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixC.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixC.cpp
$(ODIRLibFgBase)FgMatrixSolver.o: $(SDIRLibFgBase)FgMatrixSolver.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixSolver.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixSolver.cpp
$(ODIRLibFgBase)FgMatrixSolverEigen.o: $(SDIRLibFgBase)FgMatrixSolverEigen.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixSolverEigen.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixSolverEigen.cpp
$(ODIRLibFgBase)FgMatrixV.o: $(SDIRLibFgBase)FgMatrixV.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMatrixV.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMatrixV.cpp
$(ODIRLibFgBase)FgMetaFormat.o: $(SDIRLibFgBase)FgMetaFormat.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgMetaFormat.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgMetaFormat.cpp
$(ODIRLibFgBase)FgNc.o: $(SDIRLibFgBase)FgNc.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgNc.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgNc.cpp
$(ODIRLibFgBase)FgOut.o: $(SDIRLibFgBase)FgOut.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgOut.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgOut.cpp
$(ODIRLibFgBase)FgParse.o: $(SDIRLibFgBase)FgParse.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgParse.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgParse.cpp
$(ODIRLibFgBase)FgPath.o: $(SDIRLibFgBase)FgPath.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgPath.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgPath.cpp
$(ODIRLibFgBase)FgPlatform.o: $(SDIRLibFgBase)FgPlatform.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgPlatform.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgPlatform.cpp
$(ODIRLibFgBase)FgQuaternion.o: $(SDIRLibFgBase)FgQuaternion.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgQuaternion.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgQuaternion.cpp
$(ODIRLibFgBase)FgRandom.o: $(SDIRLibFgBase)FgRandom.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgRandom.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgRandom.cpp
$(ODIRLibFgBase)FgRayCaster.o: $(SDIRLibFgBase)FgRayCaster.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgRayCaster.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgRayCaster.cpp
$(ODIRLibFgBase)FgSampler.o: $(SDIRLibFgBase)FgSampler.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSampler.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSampler.cpp
$(ODIRLibFgBase)FgSerial.o: $(SDIRLibFgBase)FgSerial.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSerial.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSerial.cpp
$(ODIRLibFgBase)FgSimilarity.o: $(SDIRLibFgBase)FgSimilarity.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSimilarity.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSimilarity.cpp
$(ODIRLibFgBase)FgSoftRender.o: $(SDIRLibFgBase)FgSoftRender.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSoftRender.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSoftRender.cpp
$(ODIRLibFgBase)FgStdExtensions.o: $(SDIRLibFgBase)FgStdExtensions.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStdExtensions.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStdExtensions.cpp
$(ODIRLibFgBase)FgStdio.o: $(SDIRLibFgBase)FgStdio.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStdio.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStdio.cpp
$(ODIRLibFgBase)FgStdStream.o: $(SDIRLibFgBase)FgStdStream.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStdStream.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStdStream.cpp
$(ODIRLibFgBase)FgStdString.o: $(SDIRLibFgBase)FgStdString.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStdString.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStdString.cpp
$(ODIRLibFgBase)FgString.o: $(SDIRLibFgBase)FgString.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgString.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgString.cpp
$(ODIRLibFgBase)FgStringTest.o: $(SDIRLibFgBase)FgStringTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgStringTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgStringTest.cpp
$(ODIRLibFgBase)FgSyntax.o: $(SDIRLibFgBase)FgSyntax.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgSyntax.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgSyntax.cpp
$(ODIRLibFgBase)FgTcpTest.o: $(SDIRLibFgBase)FgTcpTest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTcpTest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTcpTest.cpp
$(ODIRLibFgBase)FgTestUtils.o: $(SDIRLibFgBase)FgTestUtils.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTestUtils.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTestUtils.cpp
$(ODIRLibFgBase)FgTime.o: $(SDIRLibFgBase)FgTime.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTime.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTime.cpp
$(ODIRLibFgBase)FgTopology.o: $(SDIRLibFgBase)FgTopology.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)FgTopology.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)FgTopology.cpp
$(ODIRLibFgBase)jpeg_mem_dest.o: $(SDIRLibFgBase)jpeg_mem_dest.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)jpeg_mem_dest.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)jpeg_mem_dest.cpp
$(ODIRLibFgBase)jpeg_mem_src.o: $(SDIRLibFgBase)jpeg_mem_src.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)jpeg_mem_src.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)jpeg_mem_src.cpp
$(ODIRLibFgBase)MurmurHash2.o: $(SDIRLibFgBase)MurmurHash2.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)MurmurHash2.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)MurmurHash2.cpp
$(ODIRLibFgBase)portable_binary_iarchive.o: $(SDIRLibFgBase)portable_binary_iarchive.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)portable_binary_iarchive.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)portable_binary_iarchive.cpp
$(ODIRLibFgBase)portable_binary_oarchive.o: $(SDIRLibFgBase)portable_binary_oarchive.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)portable_binary_oarchive.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)portable_binary_oarchive.cpp
$(ODIRLibFgBase)stdafx.o: $(SDIRLibFgBase)stdafx.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)stdafx.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)stdafx.cpp
$(ODIRLibFgBase)nix_FgClusterNix.o: $(SDIRLibFgBase)nix/FgClusterNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgClusterNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgClusterNix.cpp
$(ODIRLibFgBase)nix_FgConioNix.o: $(SDIRLibFgBase)nix/FgConioNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgConioNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgConioNix.cpp
$(ODIRLibFgBase)nix_FgFileSystemNix.o: $(SDIRLibFgBase)nix/FgFileSystemNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgFileSystemNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgFileSystemNix.cpp
$(ODIRLibFgBase)nix_FgGuiNix.o: $(SDIRLibFgBase)nix/FgGuiNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgGuiNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgGuiNix.cpp
$(ODIRLibFgBase)nix_FgSystemInfoNix.o: $(SDIRLibFgBase)nix/FgSystemInfoNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgSystemInfoNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgSystemInfoNix.cpp
$(ODIRLibFgBase)nix_FgTcpNix.o: $(SDIRLibFgBase)nix/FgTcpNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgTcpNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgTcpNix.cpp
$(ODIRLibFgBase)nix_FgTimeNix.o: $(SDIRLibFgBase)nix/FgTimeNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgTimeNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgTimeNix.cpp
$(ODIRLibFgBase)nix_FgWinSpecificNix.o: $(SDIRLibFgBase)nix/FgWinSpecificNix.cpp $(INCSLibFgBase)
	$(CXX) -o $(ODIRLibFgBase)nix_FgWinSpecificNix.o -c $(CXXFLAGS) $(FLAGSLibFgBase) $(SDIRLibFgBase)nix/FgWinSpecificNix.cpp
.PHONY: clean cleanObjs cleanTargs
clean:
	rm -r $(BUILDIR)
