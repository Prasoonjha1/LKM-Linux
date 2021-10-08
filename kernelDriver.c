#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#include <linux/mutex.h>

#define  DEVICE_NAME "kernelDriver"    ///< The device will appear at /dev/kernelDriver using this value
#define  CLASS_NAME  "char"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Prasoon Jha");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("OS Project");  ///< The description -- see modinfo
MODULE_VERSION("1.0");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  driverClass  = NULL; ///< The device-driver class struct pointer
static struct device* driverDevice = NULL; ///< The device-driver device struct pointer

static DEFINE_MUTEX(mutex_1);	   //Declaring a Mutex mutex_initialize

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

static int __init ebbchar_init(void)
{
   printk(KERN_INFO "KDriver: Initializing the KDriver LKM\n");

   // allocate a major number for the device 
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0)
   {
      printk(KERN_ALERT "KDriver failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "KDriver: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   driverClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(driverClass))	//Check for error and clean up if there is
   {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(driverClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "KDriver: device class registered correctly\n");

   // Register the device driver
   driverDevice = device_create(driverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

   if (IS_ERR(driverDevice))		//Clean up if there is an error
   {
      class_destroy(driverClass);
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(driverDevice);
   }
   printk(KERN_INFO "KDriver: device class created correctly\n"); // Made it! device was initialized

   mutex_init(&mutex_1);		//Initialize the mutex 

   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit ebbchar_exit(void)
{
   mutex_destroy(&mutex_1);	//Destroy the allocated mutex

   device_destroy(driverClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(driverClass);                          // unregister the device class
   class_destroy(driverClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "KDriver: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep)
{
   mutex_lock(&mutex_1);	//Lock the mutex and enter the critical region

   numberOpens++;
   printk(KERN_INFO "KDriver: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   if (error_count==0)		// if true then have success
   {
      printk(KERN_INFO "KDriver: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else 
   {
      printk(KERN_INFO "KDriver: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}


static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "KDriver: Received %zu characters from the user\n", len);
   return len;
}

static int dev_release(struct inode *inodep, struct file *filep)
{
   mutex_unlock(&mutex_1);	//unlock the mutex and allow anothe process to enter the critical region

   printk(KERN_INFO "KDriver: Device successfully closed\n");
   return 0;
}


module_init(ebbchar_init);
module_exit(ebbchar_exit);
