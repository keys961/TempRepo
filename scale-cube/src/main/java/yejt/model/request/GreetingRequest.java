package yejt.model.request;

public class GreetingRequest
{
    private String request;

    public GreetingRequest() {}

    public String getRequest() {
        return request;
    }

    public void setRequest(String request) {
        this.request = request;
    }

    public GreetingRequest(String request)
    {
        this.request = request;
    }
}
