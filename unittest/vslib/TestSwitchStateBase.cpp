#include <gtest/gtest.h>

#include <cstdint>
#include <vector>
#include <set>
#include <algorithm>

#include "Globals.h"
#include "sai_serialize.h"

#include "RealObjectIdManager.h"
#include "ContextConfigContainer.h"

#include "SwitchStateBase.h"

using namespace saimeta;
using namespace saivs;

class SwitchStateBaseTest : public ::testing::Test
{
public:
    SwitchStateBaseTest() = default;
    virtual ~SwitchStateBaseTest() = default;

public:
    virtual void SetUp() override
    {
        m_ccc = ContextConfigContainer::getDefault();
        m_cc = m_ccc->get(m_guid);
        m_scc = m_cc->m_scc;
        m_sc = m_scc->getConfig(m_scid);

        m_ridmgr = std::make_shared<RealObjectIdManager>(m_cc->m_guid, m_cc->m_scc);
        m_swid = m_ridmgr->allocateNewSwitchObjectId(Globals::getHardwareInfo(0, nullptr));
        m_ss = std::make_shared<SwitchStateBase>(m_swid, m_ridmgr, m_sc);
    }

    virtual void TearDown() override
    {
        // Empty
    }

protected:
    std::shared_ptr<ContextConfigContainer> m_ccc;
    std::shared_ptr<ContextConfig> m_cc;
    std::shared_ptr<SwitchConfigContainer> m_scc;
    std::shared_ptr<SwitchConfig> m_sc;
    std::shared_ptr<RealObjectIdManager> m_ridmgr;
    std::shared_ptr<SwitchStateBase> m_ss;

    sai_object_id_t m_swid = SAI_NULL_OBJECT_ID;

    const std::uint32_t m_guid = 0; // default context config id
    const std::uint32_t m_scid = 0; // default switch config id
};

TEST_F(SwitchStateBaseTest, switchHash)
{
    ASSERT_EQ(m_ss->create_default_hash(), SAI_STATUS_SUCCESS);

    sai_attribute_t attr;
    attr.id = SAI_SWITCH_ATTR_ECMP_HASH;
    ASSERT_EQ(m_ss->get(SAI_OBJECT_TYPE_SWITCH, sai_serialize_object_id(m_swid), 1, &attr), SAI_STATUS_SUCCESS);

    const auto ecmpHashOid = attr.value.oid;
    ASSERT_NE(ecmpHashOid, SAI_NULL_OBJECT_ID);

    attr.id = SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST;
    attr.value.s32list.list = nullptr;
    attr.value.s32list.count = 0;
    ASSERT_EQ(m_ss->get(SAI_OBJECT_TYPE_HASH, sai_serialize_object_id(ecmpHashOid), 1, &attr), SAI_STATUS_SUCCESS);

    std::vector<sai_int32_t> hfList(attr.value.s32list.count);
    attr.value.s32list.list = hfList.data();
    ASSERT_EQ(m_ss->get(SAI_OBJECT_TYPE_HASH, sai_serialize_object_id(ecmpHashOid), 1, &attr), SAI_STATUS_SUCCESS);

    const std::set<sai_native_hash_field_t> hfSet1 = {
        SAI_NATIVE_HASH_FIELD_DST_MAC,
        SAI_NATIVE_HASH_FIELD_SRC_MAC,
        SAI_NATIVE_HASH_FIELD_ETHERTYPE,
        SAI_NATIVE_HASH_FIELD_IN_PORT
    };

    std::set<sai_native_hash_field_t> hfSet2;

    std::transform(
        hfList.cbegin(), hfList.cend(), std::inserter(hfSet2, hfSet2.begin()),
        [](sai_int32_t value) { return static_cast<sai_native_hash_field_t>(value); }
    );
    ASSERT_EQ(hfSet1, hfSet2);
}

TEST_F(SwitchStateBaseTest, switchHashCapabilities)
{
    sai_s32_list_t data = { .count = 0, .list = nullptr };

    auto status = m_ss->queryAttrEnumValuesCapability(
        m_swid, SAI_OBJECT_TYPE_HASH, SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST, &data
    );
    ASSERT_EQ(status, SAI_STATUS_BUFFER_OVERFLOW);

    std::vector<sai_int32_t> hfList(data.count);
    data.list = hfList.data();

    status = m_ss->queryAttrEnumValuesCapability(
        m_swid, SAI_OBJECT_TYPE_HASH, SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST, &data
    );
    ASSERT_EQ(status, SAI_STATUS_SUCCESS);

    const std::set<sai_native_hash_field_t> hfSet1 = {
        SAI_NATIVE_HASH_FIELD_IN_PORT,
        SAI_NATIVE_HASH_FIELD_DST_MAC,
        SAI_NATIVE_HASH_FIELD_SRC_MAC,
        SAI_NATIVE_HASH_FIELD_ETHERTYPE,
        SAI_NATIVE_HASH_FIELD_VLAN_ID,
        SAI_NATIVE_HASH_FIELD_IP_PROTOCOL,
        SAI_NATIVE_HASH_FIELD_DST_IP,
        SAI_NATIVE_HASH_FIELD_SRC_IP,
        SAI_NATIVE_HASH_FIELD_L4_DST_PORT,
        SAI_NATIVE_HASH_FIELD_L4_SRC_PORT,
        SAI_NATIVE_HASH_FIELD_INNER_DST_MAC,
        SAI_NATIVE_HASH_FIELD_INNER_SRC_MAC,
        SAI_NATIVE_HASH_FIELD_INNER_ETHERTYPE,
        SAI_NATIVE_HASH_FIELD_INNER_IP_PROTOCOL,
        SAI_NATIVE_HASH_FIELD_INNER_DST_IP,
        SAI_NATIVE_HASH_FIELD_INNER_SRC_IP,
        SAI_NATIVE_HASH_FIELD_INNER_L4_DST_PORT,
        SAI_NATIVE_HASH_FIELD_INNER_L4_SRC_PORT
    };

    std::set<sai_native_hash_field_t> hfSet2;

    std::transform(
        hfList.cbegin(), hfList.cend(), std::inserter(hfSet2, hfSet2.begin()),
        [](sai_int32_t value) { return static_cast<sai_native_hash_field_t>(value); }
    );
    ASSERT_EQ(hfSet1, hfSet2);
}
