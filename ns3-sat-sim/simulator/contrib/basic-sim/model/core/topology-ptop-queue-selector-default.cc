#include "topology-ptop-queue-selector-default.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (TopologyPtopQueueSelectorDefault);
    TypeId TopologyPtopQueueSelectorDefault::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologyPtopQueueSelectorDefault")
                .SetParent<TopologyPtopQueueSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    /*
     * Parse the queue value into a queue object factory.
     *
     * @param value     String value (e.g., "drop_tail(100p)", "drop_tail(100000B)")
     *
     * @return Pair of (Queue object factory, Maximum queue size) if success, else throws an exception
     */
    std::pair<ObjectFactory, QueueSize> TopologyPtopQueueSelectorDefault::ParseQueueValue(Ptr<TopologyPtop>, std::string value) {

        // First-in-first-out = drop-tail
        if (starts_with(value, "drop_tail(") && ends_with(value, ")")) {

            // Get rid of the drop_tail( and ) part
            std::string fifo_max_queue_size_value = value.substr(10, value.size() - 11);

            // Make sure it is either "100p" or "100000B'
            if (!ends_with(fifo_max_queue_size_value, "p") && !ends_with(fifo_max_queue_size_value, "B")) {
                throw std::runtime_error(
                        "Invalid maximum drop-tail queue size value: " + fifo_max_queue_size_value);
            }
            parse_geq_one_int64(fifo_max_queue_size_value.substr(0, fifo_max_queue_size_value.size() - 1));

            // Finally create the queue factory
            ObjectFactory queueFactory;
            queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
            queueFactory.Set("MaxSize", QueueSizeValue(QueueSize(fifo_max_queue_size_value)));
            return std::make_pair(queueFactory, QueueSize(fifo_max_queue_size_value));

        } else {
            throw std::runtime_error("Invalid queue value: " + value);
        }

    }

}
