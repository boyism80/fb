#ifndef __MODEL_LOADER_H__
#define __MODEL_LOADER_H__

#include <fb/parallel_worker.h>
#include <fb/model/model.h>

namespace fb::model {

class context;

/**
 * @brief      This class describes a model loader.
 */
class loader : public fb::parallel_worker<std::reference_wrapper<fb::model::container>>
{
public:
    using input_type = std::reference_wrapper<fb::model::container>;

private:
    fb::model::model& _model;

public:
    /**
     * @brief      Constructs a new instance.
     *
     * @param      context  The context
     */
    loader(fb::model::model& data) : _model(data)
    {}

    /**
     * @brief      Destroys the object.
     */
    ~loader() = default;

protected:
    /**
     * @brief      Called on ready.
     *
     * @return     { description_of_the_return_value }
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
     * @brief      Called on work.
     *
     * @param[in]  value  The value
     */
    void on_work(const input_type& value)
    {
        value.get().load();
    }
    
    /**
     * @brief      Called when worked.
     *
     * @param[in]  input    The input
     * @param[in]  percent  The percent
     */
    void on_worked(const input_type& input, double percent)
    {
        fb::console::put("* [{:0.2f}%] 데이터를 읽었습니다.", percent);
    }

    /**
     * @brief      Called on error.
     *
     * @param[in]  input  The input
     * @param      e      { parameter_description }
     */
    void on_error(const input_type& input, std::exception& e)
    {
        fb::console::comment("    - {}", e.what());
    }

    /**
     * @brief      Called on finish.
     */
    void on_finish()
    {
        fb::console::next();
    }
};

}

#endif