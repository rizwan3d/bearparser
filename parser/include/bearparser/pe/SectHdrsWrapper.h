#pragma once

#include "PENodeWrapper.h"
#include <vector>
#include <map>

class SectionHdrWrapper : public PENodeWrapper
{
public:
    static const size_t SECNAME_LEN;

    static std::vector<DWORD> splitCharacteristics(DWORD characteristics);
    static QString getSecHdrAccessRightsDesc(DWORD characteristics);

    static void initSecCharacter(std::map<DWORD, QString> &secHdrCharact);
    static QString translateCharacteristics(DWORD charact);

    /* fields :*/
    enum SecFieldId {
        NAME = 0,
        VSIZE,
        VPTR,

        RSIZE,
        RPTR,

        RELOC_PTR,
        LINENUM_PTR,

        RELOC_NUM,
        LINENUM_NUM,

        CHARACT,
        FIELD_COUNTER
    };

    SectionHdrWrapper(PEFile *pe, size_t sectionNumber)
        : PENodeWrapper(pe), sectNum(sectionNumber), name(NULL), header(NULL) { wrap(); }
        
    ~SectionHdrWrapper()
    { 
        if (name) {
            ::free(name);
        }
    }

    bool wrap();

    /* full structure boundaries */
    virtual void* getPtr();
    virtual bufsize_t getSize();
    virtual QString getName();
    virtual size_t getFieldsCount() { return FIELD_COUNTER; }
    virtual size_t getSubFieldsCount() { return 1; }

    /* specific field boundaries */
    virtual void* getFieldPtr(size_t fieldId, size_t subField = FIELD_NONE);
    virtual QString getFieldName(size_t fieldId);

    virtual Executable::addr_type containsAddrType(size_t fieldId, size_t subField = FIELD_NONE);
    virtual WrappedValue::data_type containsDataType(size_t fieldId, size_t subField = FIELD_NONE);

    offset_t getContentOffset(Executable::addr_type aType, bool useMapped = true);
    offset_t getContentEndOffset(Executable::addr_type aType, bool roundup); //always useMapped startOFfset

    bufsize_t getContentSize(Executable::addr_type aType, bool recalculate);

    /* wrappers */
    DWORD getCharacteristics() { return header ? header->Characteristics : 0; }

    DWORD getRawPtr() { return header ? header->PointerToRawData : 0; }

    DWORD getVirtualPtr() { return header ? header->VirtualAddress : 0; }

//modifications:

    bool setCharacteristics(DWORD newCharacteristics)
    {
        if (!header) return false;
        //TODO: validate newCharacteristics

        header->Characteristics = newCharacteristics;
        return true;
    }

    bool reloadName();

    QString mappedName;

protected:
    SectionHdrWrapper(PEFile *pe, IMAGE_SECTION_HEADER *v_header) //standalone entry
        : PENodeWrapper(pe), sectNum(INVALID_ENTRYNUM), name(NULL), header(v_header)
    {
        reloadName();
    }

    offset_t getContentDeclaredOffset(Executable::addr_type aType);

    bufsize_t getContentDeclaredSize(Executable::addr_type aType);
    bufsize_t getMappedRawSize();
    bufsize_t getMappedVirtualSize();

    char *name;
    size_t sectNum;

private:
    static std::map<DWORD, QString> s_secHdrCharact;
    IMAGE_SECTION_HEADER *header;

friend class PEFile;
};

//----

class SectHdrsWrapper : public PENodeWrapper
{
public:
    static size_t SECT_COUNT_MAX;
    static size_t SECT_INVALID_INDEX;

    // fields :
    SectHdrsWrapper(PEFile *pe) : PENodeWrapper(pe) { wrap(); }
    bool wrap();
    virtual void reloadMapping();

    // full structure boundaries
    virtual void* getPtr();
    virtual bufsize_t getSize();
    virtual QString getName() { return "Section Hdrs"; }
    virtual size_t getFieldsCount();

    // specific field boundaries
    virtual void* getFieldPtr(size_t fieldId, size_t subField) { return getSubfieldPtr(fieldId, subField ); }
    virtual bufsize_t getFieldSize(size_t fieldId, size_t subField) { return getSubfieldSize(fieldId, subField ); }
    virtual QString getFieldName(size_t fieldId);

    SectionHdrWrapper* getSecHdrAtOffset(offset_t offset, Executable::addr_type addrType, bool roundup, bool verbose = false);

    void printSectionsMapping(Executable::addr_type aType);

    size_t getSecIndex(SectionHdrWrapper* sec) const
    {
        if (!sec || !entries.size()) return SECT_INVALID_INDEX;
        size_t indx = 0;
        for (auto itr = entries.begin(); itr != entries.end(); ++itr, ++indx) {
            if (sec == *itr) {
                return indx;
            }
        }
        return indx;
    }

    SectionHdrWrapper* getSecHdr(size_t index) const
    {
        if (index == SECT_INVALID_INDEX || index >= entries.size()) return NULL;
        return dynamic_cast<SectionHdrWrapper*>(entries.at(index));
    }

    //---
    bool canAddEntry();
    ExeNodeWrapper* addEntry(ExeNodeWrapper *entry);

protected:
    void clear();
    void addMapping(SectionHdrWrapper *sec);
    virtual bool loadNextEntry(size_t entryNum);
    bool isMyEntryType(ExeNodeWrapper *entry); // is it an entry of appropriate type

    std::map<offset_t, SectionHdrWrapper*> vSec;
    std::map<offset_t, SectionHdrWrapper*> rSec;
};
