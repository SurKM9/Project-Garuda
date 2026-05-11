/**
 * @file SpscQueue.hpp
 * @brief Lock-free single-producer single-consumer ring buffer.
 */

#ifndef SPSC_QUEUE_HPP
#define SPSC_QUEUE_HPP

#include <atomic>
#include <cstddef>

/**
 * @class SpscQueue
 * @brief A lock-free ring buffer for passing data between exactly two threads.
 *
 * The producer (writer) and consumer (reader) each own one index and never
 * contend on the same slot. std::atomic acquire/release ordering replaces
 * the need for a mutex.
 *
 * @tparam T  The element type to store (e.g. TelemetryPacket).
 * @tparam N  Total number of slots. Usable capacity is N-1.
 */
template<typename T, size_t N>
class SpscQueue {

public:

    /**
     * @brief Writes an item into the next available slot.
     * @param item  The value to copy into the buffer.
     * @return true if the item was queued, false if the buffer was full.
     */
    bool push(const T& item)
    {
        // Read our own index — relaxed is safe because only the writer ever modifies m_writeIdx
        size_t write = m_writeIdx.load(std::memory_order_relaxed);

        // Calculate where the index will land after this write, wrapping around the ring
        size_t next = (write + 1) % N;

        // Check if buffer is full — acquire pairs with reader's release so we see its latest position
        if (next == m_readIdx.load(std::memory_order_acquire))
            return false;

        // Write data into the current slot — must happen BEFORE we publish the new index
        m_slots[write] = item;

        // Publish the new write index — release guarantees slot data is visible before this store
        m_writeIdx.store(next, std::memory_order_release);
        return true;
    }

    /**
     * @brief Reads the oldest item from the buffer.
     * @param item  Output reference filled with the consumed value.
     * @return true if an item was consumed, false if the buffer was empty.
     */
    bool pop(T& item)
    {
        // Read our own index — relaxed is safe because only the reader ever modifies m_readIdx
        size_t read = m_readIdx.load(std::memory_order_relaxed);

        // Check if buffer is empty — acquire pairs with writer's release so we see completed slot data
        if (read == m_writeIdx.load(std::memory_order_acquire))
            return false;

        // Read data from the slot — acquire above guarantees the writer has finished writing it
        item = m_slots[read];

        // Advance read index, wrapping around the ring — release signals slot is free for writer to reuse
        m_readIdx.store((read + 1) % N, std::memory_order_release);
        return true;
    }

private:

    T                    m_slots[N];       ///< Fixed-size ring of packet slots
    std::atomic<size_t>  m_writeIdx{0};    ///< Write cursor — owned exclusively by the producer thread
    std::atomic<size_t>  m_readIdx{0};     ///< Read cursor — owned exclusively by the consumer thread
};

#endif /** SPSC_QUEUE_HPP **/