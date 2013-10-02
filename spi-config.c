/*
 *  linux/arch/arm/mach-bcm2708/spi_config.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define DRV_NAME        "spi_config"
#define DEFAULT_SPEED  500000
#define MAX_DEVICES 16

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/irq.h>

#include <linux/string.h>
#include <linux/spi/spi.h>

#include <linux/can/platform/mcp251x.h>

/* the module parameters */
static char* devices="";
module_param(devices, charp, S_IRUGO);
MODULE_PARM_DESC(devices, "SPI device configs");
/* the devices that we have registered */
static struct spi_device *spi_devices[MAX_DEVICES];
static int spi_devices_count=0;

static void register_device(char *devdesc);
static void release_device(struct spi_device * dev);

static int __init spi_config_init(void)
{
	char *head=devices;
	char *idx=NULL;
	/* clean up the spi_devices array */
	memset(spi_devices,0,sizeof(spi_devices));
	/* parse the devices parameter */
	while(*head) {
		/* find delimiter and create a separator */
		idx=strchr(head,',');if (idx) { *idx=0; }
		/* now parse the argument - if it is not "empty" */
		if (*head) { register_device(head); }
		/* and skip to next section and contine - exiting if there was no more ","*/
		if (idx) { head=idx+1; } else { break;}
	}
	/* and return OK */
        return 0;
}
module_init(spi_config_init);

static void __exit spi_config_exit(void)
{
	int i;
	/* unregister devices */
	for(i=0;i<MAX_DEVICES;i++) {
		if (spi_devices[i]) { 
			release_device(spi_devices[i]); 
			spi_devices[i]=NULL;
		}
	}
	/* and return */
	return;
}
module_exit(spi_config_exit);

static void register_device(char *devdesc) {
	char *tmp;
	int i;
	/* some of the objects we create/need */
	struct spi_board_info *brd;
	struct spi_master *master;
	/* some variables for later use */
	int bus,cs;
	char *modalias=NULL;
	int max_speed_hz=DEFAULT_SPEED;
	int gpio=-1,irq=-1;
	/* log the parameter */
	printk(KERN_INFO "spi_config_register: description: %s\n", devdesc);
	/* get the busid */
	tmp=strsep(&devdesc,":");
	if (kstrtoint(tmp,10,&bus)) {
		printk(KERN_ERR " spi_config_register: bus_id %s can not get parsed - ignoring config\n",tmp);
		return;
	}
	/* get the cs */
	if (!devdesc) { 
		printk(KERN_INFO " spi_config_register: minimum requirement (bus_id:cs_id:modalias[:speed][:irq][:extra]) for description not fullfilled - ignoring\n");
		return;
	}
	tmp=strsep(&devdesc,":");
	if (kstrtoint(tmp,10,&cs)) {
		printk(KERN_ERR " spi_config_register: cs_id %s can not get parsed - ignoring config\n",tmp);
		return;
	}
	/* get the driver name */
	if (!devdesc) { 
		printk(KERN_INFO " spi_config_register: minimum requirement (bus_id:cs_id:modalias[:speed][:irq][:extra]) for description not fullfilled - ignoring\n");
		return;
	}
	modalias=strsep(&devdesc,":");
	/* check that driver name is not empty */
	if (!*modalias) {
		printk(KERN_ERR " spi_config_register: driver name \"%s\" not valid (empty) - ignoring config\n",tmp);
		return;
	}
	/* get the max_speed_hz parmeter - if we have it */
	tmp=strsep(&devdesc,":");
	if (tmp) {
		if (kstrtoint(tmp,10,&max_speed_hz)) {
			printk(KERN_ERR " spi_config_register: max_speed_hz %s can not get parsed - ignoring config\n",tmp);
			return;
		}
		
	}
	/* and the IRQ parameter */
	tmp=strsep(&devdesc,":");
	if (tmp) {
		if (kstrtoint(tmp,10,&gpio)) {
			printk(KERN_ERR " spi_config_register: irq %s can not get parsed - ignoring config\n",tmp);
			return;
		}
		irq=gpio_to_irq(gpio);
	}

	/* check that there is no driver yet for that bus/cs registered */
	for(i=0;i<spi_devices_count;i++) {
		if (
			(spi_devices[i]) 
			&& (bus==spi_devices[i]->master->bus_num)
			&& (cs==spi_devices[i]->chip_select)
			) {
			printk(KERN_ERR " spi_config_register: spi%i.%i allready assigned - ignoring config\n",bus,cs);
			return;
		}
	}
	/* get the bus master */
	master=spi_busnum_to_master(bus);
	if (!master) {
		printk(KERN_ERR " spi_config_register: no spi%i bus found - ignoring config\n",bus);
		return;
	}
	/* now start assigning stuff - creating a device */
	brd=kmalloc(sizeof(struct spi_board_info),GFP_KERNEL);
	memset(brd,0,sizeof(struct spi_board_info));
	brd->max_speed_hz=max_speed_hz;
        brd->bus_num=bus;
        brd->chip_select=cs;
        brd->mode=SPI_MODE_0;
	brd->irq=irq;
	strncpy(brd->modalias,modalias,sizeof(brd->modalias));
	/* and now check the extra data */
	if ((strcmp(brd->modalias,"mcp2515")==0)||(strcmp(brd->modalias,"mcp251x")==0)) {
		struct mcp251x_platform_data *pd=kmalloc(sizeof(struct mcp251x_platform_data),GFP_KERNEL);
		memset(pd,0,sizeof(struct mcp251x_platform_data));
		pd->irq_flags = IRQF_TRIGGER_FALLING|IRQF_ONESHOT;
		pd->oscillator_frequency = 16000000;
		brd->platform_data=pd;
	}
	/* and register it */
	if ((spi_devices[spi_devices_count]=spi_new_device(master,brd))) {
		if (spi_devices[spi_devices_count]->irq<0) {
			printk(KERN_INFO "spi_config_register:spi%i.%i: registering modalias=%s with max_speed_hz=%i and no interrupt\n",
				spi_devices[spi_devices_count]->master->bus_num,
				spi_devices[spi_devices_count]->chip_select,
				spi_devices[spi_devices_count]->modalias,
				spi_devices[spi_devices_count]->max_speed_hz
				);
		} else {
			printk(KERN_INFO "spi_config_register:spi%i.%i: registering modalias=%s with max_speed_hz=%i and gpio/irq=%i/%i\n",
				spi_devices[spi_devices_count]->master->bus_num,
				spi_devices[spi_devices_count]->chip_select,
				spi_devices[spi_devices_count]->modalias,
				spi_devices[spi_devices_count]->max_speed_hz,
				gpio,
				spi_devices[spi_devices_count]->irq
				);
		}
		if (spi_devices[spi_devices_count]->dev.platform_data) {
			printk(KERN_INFO "spi_config_register:spi%i.%i:%s: platform data=%32ph\n",
				spi_devices[spi_devices_count]->master->bus_num,
				spi_devices[spi_devices_count]->chip_select,
				spi_devices[spi_devices_count]->modalias,
				spi_devices[spi_devices_count]->dev.platform_data
				);
		}
		spi_devices_count++;
	} else {
		printk(KERN_ERR "spi_config_register:spi%i.%i:%s: failed to register device\n", bus,cs,modalias);
		/* on error clean up */
		if (brd->platform_data) kfree(brd->platform_data);
		kfree(brd);
		/* clean also the entry */
		spi_devices[spi_devices_count]=NULL;
		return;
	}
	/* and return */
	return;
}

static void release_device(struct spi_device *spi) {
	printk(KERN_INFO "spi_config_unregister:spi%i.%i: unregister device with modalias %s\n", spi->master->bus_num,spi->chip_select,spi->modalias);
	/* unregister device */
	spi_unregister_device(spi);
	/* seems as if unregistering also means that the structures get freed as well - kernel crashes, so we do not do it */
}

/* the module description */
MODULE_DESCRIPTION("SPI board setup");
MODULE_AUTHOR("Martin Sperl <kernel@martin.sperl.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);

