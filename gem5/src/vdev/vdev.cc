//
// Created by lf-z on 4/24/17.
//

#include "vdev/vdev.hh"
#include "debug/EnergyMgmt.hh"
#include "debug/VirtualDevice.hh"
#include "debug/MemoryAccess.hh"

VirtualDevice::DevicePort::DevicePort(const std::string &_name, VirtualDevice *_vdev)
    : SlavePort(_name, _vdev), vdev(_vdev)
{

}

Tick
VirtualDevice::DevicePort::recvAtomic(PacketPtr pkt)
{
    assert(vdev);
    return vdev->recvAtomic(pkt);
}

void
VirtualDevice::DevicePort::recvFunctional(PacketPtr pkt)
{
    assert(vdev);
    vdev->recvFunctional(pkt);
}

bool
VirtualDevice::DevicePort::recvTimingReq(PacketPtr pkt)
{
    assert(vdev);
    return vdev->recvTimingReq(pkt);
}

void
VirtualDevice::DevicePort::recvRespRetry()
{
    assert(vdev);
    vdev->recvRespRetry();
}

AddrRangeList
VirtualDevice::DevicePort::getAddrRanges() const
{
    assert(vdev);

    AddrRangeList rangeList;
    rangeList.push_back(vdev->getAddrRange());

    return rangeList;
}

VirtualDevice::VirtualDevice(const Params *p)
    : MemObject(p),
      id(0),
      port(name() + ".port", this),
      cpu(p->cpu),
      range(p->range),
      delay_set(p->delay_set),
      delay_cpu_interrupt(p->delay_cpu_interrupt),
      event_interrupt(this, false, Event::Virtual_Interrupt)
{
    trace.resize(0);
    pmem = (uint8_t*) malloc(range.size() * sizeof(uint8_t));
    memset(pmem, 0, range.size() * sizeof(uint8_t));
    
    aichip.tickPerCycle = clockPeriod();
}

void
VirtualDevice::init()
{
    MemObject::init();

    if (port.isConnected()) {
        port.sendRangeChange();
    }

    //cpu->registerVDev(delay_recover, id);
    cpu->registerVDev(1, id);
    DPRINTF(VirtualDevice, "Virtual Device started with range: %#lx - %#lx\n",
            range.start(), range.end());

    execution_state = STATE_POWEROFF;
}

void
VirtualDevice::triggerInterrupt()
{
    if(aichip.finished)
    {
      DPRINTF(VirtualDevice, "Virtual device triggers an interrupt.\n");
      execution_state = STATE_IDLE; // The virtual device enter/keep in the active status.
      finishSuccess();
      assert(*pmem & VDEV_WORK);
      /* Change register byte. */
      *pmem |= VDEV_FINISH;
      *pmem &= ~VDEV_WORK;

      /* Tell cpu. */
      cpu->virtualDeviceInterrupt(delay_cpu_interrupt);
      cpu->virtualDeviceEnd(id);    
      
      /*output result*/
      memcpy(pmem + 1,&aichip.result,sizeof(int));
    }
    else //not finished, run next inst
    {
      /* Schedule interrupt. */
      delay_self = aichip.run();
      schedule(event_interrupt, curTick() + delay_self);
      DPRINTF(VirtualDevice, "Next Inst, Need Ticks:%i, Cycles:%i .\n", 
        delay_self, 
        ticksToCycles(delay_self)
      );
    }
}

Tick
VirtualDevice::access(PacketPtr pkt)
{
    /* Todo: what if the cpu ask to work on a task when the vdev is busy? **/
    DPRINTF(MemoryAccess, "Virtual Device accessed at %#lx.\n", pkt->getAddr());
    Addr offset = pkt->getAddr() - range.start();
    if (pkt->isRead()) {
        memcpy(pkt->getPtr<uint8_t>(), pmem+offset, pkt->getSize());
    } else if (pkt->isWrite()) {
        const uint8_t* pkt_addr = pkt->getConstPtr<uint8_t>();
        if (offset == 0) {
            /* Might be a request. */
            if (*pkt_addr & VDEV_SET) {
                /* Request */
                if (*pmem & VDEV_WORK) {
                    /* Request fails because the vdev is working. */
                    DPRINTF(VirtualDevice, "Request discarded!\n");
                } else {
                    /* Request succeeds. */
                    execution_state = STATE_ACTIVE; // The virtual device enter/keep in the active status.
                    DPRINTF(VirtualDevice, "Virtual Device starts working.\n");                
                    
                    /* Set the virtual device to working mode */
                    *pmem |= VDEV_WORK;
                    *pmem &= ~VDEV_FINISH;
                    /* Schedule interrupt. */
                    delay_self = aichip.run();
                    schedule(event_interrupt, curTick() + delay_set + delay_self);
                    DPRINTF(VirtualDevice, "Need Ticks:%i, Cycles:%i.\n", 
                      delay_self, 
                      ticksToCycles(delay_self)
                    );
                    cpu->virtualDeviceSet(delay_set);
                    cpu->virtualDeviceStart(id);
                }
            } else {
                /* Not a request, but the first byte cannot be written. */
            }
        } else {
            /* Normal write. */
            memcpy(pmem+offset, pkt_addr, pkt->getSize());
        }
    }
    return 0;
}


void 
VirtualDevice::tick()
{
}


int
VirtualDevice::handleMsg(const EnergyMsg &msg)
{
    DPRINTF(EnergyMgmt, "Device handleMsg called at %lu, msg.type=%d\n", curTick(), msg.type);
    return 0;
}

BaseSlavePort&
VirtualDevice::getSlavePort(const std::string &if_name, PortID idx)
{
    if (if_name == "port") {
        return port;
    } else {
        return MemObject::getSlavePort(if_name, idx);
    }
}

AddrRange
VirtualDevice::getAddrRange() const
{
    return range;
}

Tick
VirtualDevice::recvAtomic(PacketPtr pkt)
{
    return access(pkt);
}

void
VirtualDevice::recvFunctional(PacketPtr pkt)
{
    fatal("Functional access is not supported now.");
}

bool
VirtualDevice::recvTimingReq(PacketPtr pkt)
{
    fatal("Timing access is not supported now.");
}

void
VirtualDevice::recvRespRetry()
{
    fatal("Timing access is not supported now.");
}

bool
VirtualDevice::finishSuccess()
{
    /* Todo: Need further implementation. */
    return 1;
}

VirtualDevice *
VirtualDeviceParams::create()
{
    return new VirtualDevice(this);
}
