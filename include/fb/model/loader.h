#ifndef __MODEL_LOADER_H__
#define __MODEL_LOADER_H__

#include <fb/parallel_worker.h>
#include <fb/model/model.h>

using namespace fb;

namespace fb::model {

class server;

/**
 * @brief      Parallel data loader for game model containers.
 *
 *             This class extends the parallel worker framework to load game data files
 *             concurrently across multiple threads. It processes model containers in
 *             parallel, providing progress feedback and error handling during the
 *             loading process. Essential for fast server startup with large datasets.
 */
class loader : public fb::parallel_worker<std::reference_wrapper<fb::model::container>>
{
public:
    using input_type = std::reference_wrapper<fb::model::container>;

private:
    fb::model::model& _model;

public:
    /**
     * @brief      Constructs a model loader for the specified data model.
     *
     * @param      data  The game data model containing containers to load.
     */
    loader(fb::model::model& data) :
        _model(data)
    { }

    /**
     * @brief      Destroys the object.
     */
    ~loader() = default;

protected:
    /**
     * @brief      Generates the list of containers to be loaded in parallel.
     *
     *             Iterates through all containers in the model and yields each one
     *             for parallel processing. This method is called once at the start
     *             of the loading process to prepare the work queue.
     *
     * @return     A generator that yields container references for processing.
     */
    fb::generator<input_type> on_ready()
    {
        auto buffer = std::vector<std::reference_wrapper<fb::model::container>>();
        this->_model.foreach ([&, this](auto& container) {
            buffer.push_back(container);
        });

        for (auto& container : buffer)
        {
            co_yield container;
        }
    }

    /**
     * @brief      Performs the actual loading work for a single container.
     *
     *             This method is called by worker threads to load data from files
     *             into the specified container. Each container handles its own
     *             file format and loading logic through the load() method.
     *
     * @param[in]  value  The container reference to load data into.
     */
    void on_work(const input_type& value)
    {
        value.get().load();
    }

    /**
     * @brief      Called after each container is successfully loaded.
     *
     *             Updates the progress display to show loading completion percentage.
     *             This provides visual feedback during the loading process.
     *
     * @param[in]  input    The container that was loaded.
     * @param[in]  percent  The completion percentage (0.0 to 100.0).
     */
    void on_worked(const input_type& input, double percent)
    {
        fb::console::progress("Loading data files", percent);
    }

    /**
     * @brief      Called when an error occurs during container loading.
     *
     *             Logs the error message to the console for debugging purposes.
     *             The loading process continues with other containers even if
     *             some containers fail to load.
     *
     * @param[in]  input  The container that failed to load.
     * @param      e      The exception that occurred during loading.
     */
    void on_error(const input_type& input, std::exception& e)
    {
        fb::console::comment("    - {}", e.what());
    }

    /**
     * @brief      Called when all containers have finished loading.
     *
     *             Adds a newline to the console output to complete the progress display.
     *             This method is called once after all parallel loading operations complete.
     */
    void on_finish()
    {
        fb::console::newline();
    }
};

} // namespace fb::model

#endif