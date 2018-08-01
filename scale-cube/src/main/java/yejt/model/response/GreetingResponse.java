package yejt.model.response;


import java.io.Serializable;

public class GreetingResponse
{
    private String response;

    public GreetingResponse() {}

    public String getResponse() {
        return response;
    }

    public void setResponse(String response) {
        this.response = response;
    }

    public GreetingResponse(String response)
    {
        this.response = response;
    }
}
