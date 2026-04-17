#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soem/soem.h"

typedef struct __attribute__((packed)) {
  uint32_t control1;   // 0x7010:01
  uint32_t control2;   // 0x7010:02
} ati_rxpdo_t;

typedef struct __attribute__((packed)) {
  int32_t fx;          // 0x6000:01
  int32_t fy;          // 0x6000:02
  int32_t fz;          // 0x6000:03
  int32_t tx;          // 0x6000:04
  int32_t ty;          // 0x6000:05
  int32_t tz;          // 0x6000:06
} ati_txpdo_t;

#define NUMBER_OF_SLAVES 1
#define IOMAP_SIZE 4096

static ecx_contextt ctx;

uint32 verify_network_configuration(ecx_contextt * ctx) {
  if (ctx->slavecount == NUMBER_OF_SLAVES) {
    return 0;
  }
  else {
    return 1;
  }
}

static int sdo_read_u8(ecx_contextt *ctx, uint16 slave, uint16 index, uint8 subidx, uint8 *val) {
    int size = sizeof(*val);
    int rc = ecx_SDOread(ctx, slave, index, subidx, FALSE, &size, val, EC_TIMEOUTRXM);
    if (rc <= 0) {
        fprintf(stderr, "SDO read failed: slave=%u idx=0x%04X sub=0x%02X\n", slave, index, subidx);
        return 0;
    }
    return 1;
}

static int sdo_read_u16(ecx_contextt *ctx, uint16 slave, uint16 index, uint8 subidx, uint16 *val) {
    int size = sizeof(*val);
    int rc = ecx_SDOread(ctx, slave, index, subidx, FALSE, &size, val, EC_TIMEOUTRXM);
    if (rc <= 0) {
        fprintf(stderr, "SDO read failed: slave=%u idx=0x%04X sub=0x%02X\n", slave, index, subidx);
        return 0;
    }
    return 1;
}

static int sdo_read_u32(ecx_contextt *ctx, uint16 slave, uint16 index, uint8 subidx, uint32 *val) {
    int size = sizeof(*val);
    int rc = ecx_SDOread(ctx, slave, index, subidx, FALSE, &size, val, EC_TIMEOUTRXM);
    if (rc <= 0) {
        fprintf(stderr, "SDO read failed: slave=%u idx=0x%04X sub=0x%02X\n", slave, index, subidx);
        return 0;
    }
    return 1;
}

static int sdo_read_s32(ecx_contextt *ctx, uint16 slave, uint16 index, uint8 subidx, int32 *val) {
    int size = sizeof(*val);
    int rc = ecx_SDOread(ctx, slave, index, subidx, FALSE, &size, val, EC_TIMEOUTRXM);
    if (rc <= 0) {
        fprintf(stderr, "SDO read failed: slave=%u idx=0x%04X sub=0x%02X\n", slave, index, subidx);
        return 0;
    }
    return 1;
}

static int sdo_read_string(ecx_contextt *ctx, uint16 slave, uint16 index, uint8 subidx, char *buf, size_t buf_sz) {
    int size = (int)buf_sz;
    memset(buf, 0, buf_sz);

    int rc = ecx_SDOread(ctx, slave, index, subidx, FALSE, &size, buf, EC_TIMEOUTRXM);
    if (rc <= 0) {
        fprintf(stderr, "SDO read failed: slave=%u idx=0x%04X sub=0x%02X\n", slave, index, subidx);
        return 0;
    }

    // make sure it is terminated even if buffer is full
    buf[buf_sz - 1] = '\0';
    return 1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage : %s <ifacename>\n", argv[0]);
    return 1;
  }

  char * iface = argv[1];

  // Sets up NIC
  if (ecx_init(&ctx, iface) <= 0) {
    fprintf(stderr, "Error: ecx_init\n");
    return 1;
  }

  // Configures the network
  if (ecx_config_init(&ctx) <= 0) {
    fprintf(stderr, "Error: ecx_config_init\n");
    return 1;
  }

  // Checks if the slavecount is equal to the expected amount
  if (verify_network_configuration(&ctx) == 1) {
    fprintf(stderr, "Error: verify_network_configuration\n");
  }

  // Maps the slaves to the IOmap
  uint8_t IOmap[IOMAP_SIZE];
  int size = ecx_config_map_group(&ctx, IOmap, 0);
  if (size > IOMAP_SIZE) {
    fprintf(stderr, "Error: iomap size too small\n");
  }

  // Calculates working counter
  int expectedWKC = ctx.grouplist[0].outputsWKC * 2 + ctx.grouplist[0].inputsWKC;

  // Configures EtherCat clock and measures propogation delay of slaves
  ecx_configdc(&ctx);

  // Requests slaves to enter safe op mode
  ecx_statecheck(&ctx, 0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

  // Ensuring that all slaves have valid outputs
  ecx_send_processdata(&ctx);
  ecx_receive_processdata(&ctx, EC_TIMEOUTRET);

  // Requesting operational state for all slaves
  ctx.slavelist[0].state = EC_STATE_OPERATIONAL;
  ecx_writestate(&ctx, 0);

  // Setting up output and input structs
  ati_rxpdo_t *out = (ati_rxpdo_t *) ctx.slavelist[1].outputs;
  ati_txpdo_t *in  = (ati_txpdo_t *) ctx.slavelist[1].inputs;

  if (out == NULL) {
    fprintf(stderr, "Slave 1 outputs pointer is NULL\n");
    return 1;
  }

  if (in == NULL) {
    fprintf(stderr, "Slave 1 inputs pointer is NULL\n");
    return 1;
  }

  out->control1 = 0;
  out->control2 = 0;


  // Checking all slaves could enter operational mode
  int chk = 40;
  do {
    ecx_send_processdata(&ctx);
    ecx_receive_processdata(&ctx, EC_TIMEOUTRET);
    ecx_statecheck(&ctx, 0, EC_STATE_OPERATIONAL, 50000);
  } while (chk-- && (ctx.slavelist[0].state != EC_STATE_OPERATIONAL));
  if (ctx.slavelist[0].state != EC_STATE_OPERATIONAL) {
    fprintf(stderr, "Failed to reach OP state. Master state = 0x%02x\n",
        ctx.slavelist[0].state);
    return 1;
  }

  // Operational state reached, starting 


    // Reading SDO Data

    uint16 slave = 1;

    // identity object 0x1018
    uint32 vendor_id = 0, product_code = 0, revision = 0, serial_num = 0;
    sdo_read_u32(&ctx, slave, 0x1018, 1, &vendor_id);
    sdo_read_u32(&ctx, slave, 0x1018, 2, &product_code);
    sdo_read_u32(&ctx, slave, 0x1018, 3, &revision);
    sdo_read_u32(&ctx, slave, 0x1018, 4, &serial_num);

    //  0x2040 calibration/settings block
    char ft_serial[9];
    char cal_part[31];
    char cal_family[9];
    char cal_time[31];
    uint8 force_units = 0, torque_units = 0;
    int32 counts_per_force = 0, counts_per_torque = 0;

    sdo_read_string(&ctx, slave, 0x2040, 1, ft_serial, sizeof(ft_serial));
    sdo_read_string(&ctx, slave, 0x2040, 2, cal_part, sizeof(cal_part));
    sdo_read_string(&ctx, slave, 0x2040, 3, cal_family, sizeof(cal_family));
    sdo_read_string(&ctx, slave, 0x2040, 4, cal_time, sizeof(cal_time));
    sdo_read_u8(&ctx, slave, 0x2040, 41, &force_units);
    sdo_read_u8(&ctx, slave, 0x2040, 42, &torque_units);
    sdo_read_s32(&ctx, slave, 0x2040, 49, &counts_per_force);
    sdo_read_s32(&ctx, slave, 0x2040, 50, &counts_per_torque);

    // 0x2020 tool transform
    int32 Rx, Ry, Rz, Dx, Dy, Dz;
    sdo_read_s32(&ctx, slave, 0x2020, 1, &Rx);
    sdo_read_s32(&ctx, slave, 0x2020, 2, &Ry);
    sdo_read_s32(&ctx, slave, 0x2020, 3, &Rz);
    sdo_read_s32(&ctx, slave, 0x2020, 4, &Dx);
    sdo_read_s32(&ctx, slave, 0x2020, 5, &Dy);
    sdo_read_s32(&ctx, slave, 0x2020, 6, &Dz);

  while (1) {
    // Keep control words zero unless command needs to be sent
    out->control1 = 0;
    out->control2 = 0;

    // If not continually sending process data watchdog will be sent
    ecx_send_processdata(&ctx);
    int wkc = ecx_receive_processdata(&ctx, EC_TIMEOUTRET);

    if (wkc >= expectedWKC) {
      printf("Fx=%d  Fy=%d  Fz=%d  Tx=%d  Ty=%d  Tz=%d\n",
          in->fx / counts_per_force, in->fy / counts_per_force, in->fz / counts_per_force,
          in->tx / counts_per_torque, in->ty / counts_per_torque, in->tz / counts_per_torque);
    }
    else {
      fprintf(stderr, "Bad WKC: got %d expected >= %d\n", wkc, expectedWKC);

      ecx_readstate(&ctx);
      for (int i = 1; i <= ctx.slavecount; i++) {
        if (ctx.slavelist[i].state != EC_STATE_OPERATIONAL) {
          fprintf(stderr,
              "Slave %d not OP. state = 0x%02x ALstatus = 0x%04x\n",
              i,
              ctx.slavelist[i].state,
              ctx.slavelist[i].ALstatuscode);
        }
      }
    }

    usleep(1000);
  }


}

