#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/module.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/route.h>
#include <linux/pkt_sched.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter/xt_tcpudp.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/if_arp.h>
#include <linux/kallsyms.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <net/protocol.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sergey Sergeev <sergey.sergeev@yapic.net>");
MODULE_DESCRIPTION("kernel test");

static int owl = 0;
module_param(owl, int, 0);
MODULE_PARM_DESC(owl, "An OwL");

#define IPQ40XX_MDIO_BASE	0x90000
#define MDIO_CTRL_0_REG		0x40
#define MDIO_CTRL_1_REG		0x44
#define MDIO_CTRL_2_REG		0x48
#define MDIO_CTRL_3_REG		0x4c
#define MDIO_CTRL_4_REG		0x50
#define MDIO_CTRL_4_ACCESS_BUSY		(1 << 16)
#define MDIO_CTRL_4_ACCESS_START	(1 << 8)
#define MDIO_CTRL_4_ACCESS_CODE_READ	0
#define MDIO_CTRL_4_ACCESS_CODE_WRITE	1
#define IPQ40XX_MDIO_RETRY	1000
#define IPQ40XX_MDIO_DELAY	5

#define QCA8075_PHY_ID1				2
#define QCA8075_PHY_ID2				3

void __iomem *mdio_base = NULL;

static int ipq40xx_mdio_wait_busy(void)
{
	int i;
	u32 busy;
	for (i = 0; i < IPQ40XX_MDIO_RETRY; i++) {
		udelay(IPQ40XX_MDIO_DELAY);
		busy = readl(mdio_base +
			MDIO_CTRL_4_REG) &
			MDIO_CTRL_4_ACCESS_BUSY;

		if (!busy)
			return 0;
		udelay(IPQ40XX_MDIO_DELAY);
	}
	printk(KERN_ERR "%s: MDIO operation timed out\n",
			__func__);
	return -ETIMEDOUT;
}

int ipq40xx_mdio_write(int mii_id, int regnum, u16 value)
{
	if (ipq40xx_mdio_wait_busy())
		return -ETIMEDOUT;
	/* Issue the phy addreass and reg */
	writel((mii_id << 8 | regnum),
		mdio_base + MDIO_CTRL_1_REG);

	/* Issue a write data */
	writel(value, mdio_base + MDIO_CTRL_2_REG);

	/* Issue write command */
	writel((MDIO_CTRL_4_ACCESS_START |
		MDIO_CTRL_4_ACCESS_CODE_WRITE),
		(mdio_base + MDIO_CTRL_4_REG));

	/* Wait for write complete */

	if (ipq40xx_mdio_wait_busy())
		return -ETIMEDOUT;

	return 0;
}

int ipq40xx_mdio_read(int mii_id, int regnum, ushort *data)
{
	u32 val;
	if (ipq40xx_mdio_wait_busy())
		return -ETIMEDOUT;

	/* Issue the phy address and reg */
	writel((mii_id << 8) | regnum,
		mdio_base + MDIO_CTRL_1_REG);

	/* issue read command */
	writel((MDIO_CTRL_4_ACCESS_START |
		MDIO_CTRL_4_ACCESS_CODE_READ),
		(mdio_base + MDIO_CTRL_4_REG));

	if (ipq40xx_mdio_wait_busy())
		return -ETIMEDOUT;

	/* Read data */
	val = readl(mdio_base + MDIO_CTRL_3_REG);

	if (data != NULL)
		*data = val;

	return val;
}

static u16 qca8075_phy_reg_write(u32 dev_id, u32 phy_id,
		u32 reg_id, u16 reg_val)
{
	ipq40xx_mdio_write(phy_id, reg_id, reg_val);
	return 0;
}

u16 qca8075_phy_reg_read(u32 dev_id, u32 phy_id, u32 reg_id)
{
	return ipq40xx_mdio_read(phy_id, reg_id, NULL);
}


//*********************************************************
//выполняется при загрузке модуля
static int __init test_m_module_init(void){
	if(1){ /* ipq4019 mdio/mii */
		mdio_base = ioremap(IPQ40XX_MDIO_BASE, 0x54);
		u16 phy_data;
		printk(KERN_INFO "ipq_mdio reg0 = 0x%x\n", readl(mdio_base + MDIO_CTRL_0_REG));
		printk(KERN_INFO "ipq_mdio reg1 = 0x%x\n", readl(mdio_base + MDIO_CTRL_1_REG));
		printk(KERN_INFO "ipq_mdio reg2 = 0x%x\n", readl(mdio_base + MDIO_CTRL_2_REG));
		printk(KERN_INFO "ipq_mdio reg3 = 0x%x\n", readl(mdio_base + MDIO_CTRL_3_REG));
		printk(KERN_INFO "ipq_mdio reg3 = 0x%x\n", readl(mdio_base + MDIO_CTRL_4_REG));
		//phy_data = qca8075_phy_reg_read(0x0, 0x1C, QCA8075_PHY_ID1);
		phy_data = qca8075_phy_reg_read(0x0, 0x00, QCA8075_PHY_ID1);
		printk(KERN_INFO "PHY ID1: 0x%x\n", phy_data);
		//phy_data = qca8075_phy_reg_read(0x0, 0x1C, QCA8075_PHY_ID2);
		phy_data = qca8075_phy_reg_read(0x0, 0x00, QCA8075_PHY_ID2);
		printk(KERN_INFO "PHY ID2: 0x%x\n", phy_data);
		iounmap(mdio_base);
		return -ENOMEM;
	}
	return 0;
}//--------------------------------------------------------

//*********************************************************
//выполняется при выгрузке модуля
static void __exit test_m_module_exit (void){
}//--------------------------------------------------------

module_init(test_m_module_init);
module_exit(test_m_module_exit);
