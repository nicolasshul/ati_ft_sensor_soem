#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "soem/soem.h"
#include "ecat.h"

static ecx_contextt ctx;

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

    uint16_t slave = 1;
    ati_sdo_t sdo_data;

    // identity object 0x1018
    ati_identity_t sdo_id = sdo_data.identity;

    sdo_read_u32(&ctx, slave, 0x1018, 1, &sdo_id.vendor_id);
    sdo_read_u32(&ctx, slave, 0x1018, 2, &sdo_id.product_code);
    sdo_read_u32(&ctx, slave, 0x1018, 3, &sdo_id.revision);
    sdo_read_u32(&ctx, slave, 0x1018, 4, &sdo_id.serial_num);

    //  0x2040 calibration/settings block
    ati_calibration_t cal = sdo_data.calibration;

    sdo_read_string(&ctx, slave, 0x2040, 1, cal.ft_serial, sizeof(cal.ft_serial));
    sdo_read_string(&ctx, slave, 0x2040, 2, cal.cal_part, sizeof(cal.cal_part));
    sdo_read_string(&ctx, slave, 0x2040, 3, cal.cal_family, sizeof(cal.cal_family));
    sdo_read_string(&ctx, slave, 0x2040, 4, cal.cal_time, sizeof(cal.cal_time));
    sdo_read_u8(&ctx, slave, 0x2040, 41, &cal.force_units);
    sdo_read_u8(&ctx, slave, 0x2040, 42, &cal.torque_units);
    sdo_read_s32(&ctx, slave, 0x2040, 49, &cal.counts_per_force);
    sdo_read_s32(&ctx, slave, 0x2040, 50, &cal.counts_per_torque);

    // 0x2020 tool transform
    ati_transform_t tf = sdo_data.transform;

    sdo_read_s32(&ctx, slave, 0x2020, 1, &tf.Rx);
    sdo_read_s32(&ctx, slave, 0x2020, 2, &tf.Ry);
    sdo_read_s32(&ctx, slave, 0x2020, 3, &tf.Rz);
    sdo_read_s32(&ctx, slave, 0x2020, 4, &tf.Dx);
    sdo_read_s32(&ctx, slave, 0x2020, 5, &tf.Dy);
    sdo_read_s32(&ctx, slave, 0x2020, 6, &tf.Dz);


    FILE *fp = fopen("data.csv", "w");
    fprintf(fp, "t,fx,fy,fz,tx,ty,tz\n");
    double t = 0.0;
    double dt = 0.001;

  while (1) {
    // Keep control words zero unless command needs to be sent
    out->control1 = 0;
    out->control2 = 0;

    // If not continually sending process data watchdog will be sent
    ecx_send_processdata(&ctx);
    int wkc = ecx_receive_processdata(&ctx, EC_TIMEOUTRET);

    if (wkc >= expectedWKC) {
      printf("Fx=%f  Fy=%f  Fz=%f  Tx=%f  Ty=%f  Tz=%f\n",
          (double) in->fx / cal.counts_per_force, (double) in->fy / cal.counts_per_force, (double) in->fz / cal.counts_per_force,
          (double) in->tx / cal.counts_per_torque, (double) in->ty / cal.counts_per_torque, (double) in->tz / cal.counts_per_torque);
      fprintf(fp, "%f,%f,%f,%f,%f,%f,%f\n", t,
          (double) in->fx / cal.counts_per_force, (double) in->fy / cal.counts_per_force, (double) in->fz / cal.counts_per_force,
          (double) in->tx / cal.counts_per_torque, (double) in->ty / cal.counts_per_torque, (double) in->tz / cal.counts_per_torque);
      t += dt;
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

  fclose(fp);

}

