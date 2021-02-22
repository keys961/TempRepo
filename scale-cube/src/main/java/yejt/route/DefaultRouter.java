package yejt.route;

import io.scalecube.services.ServiceReference;
import io.scalecube.services.api.ServiceMessage;
import io.scalecube.services.registry.api.ServiceRegistry;
import io.scalecube.services.routing.Router;

import java.util.List;
import java.util.Optional;

public class DefaultRouter implements Router
{
    @Override
    public Optional<ServiceReference> route(ServiceRegistry serviceRegistry, ServiceMessage request)
    {
        List<ServiceReference> references = serviceRegistry.lookupService(request);
        if(references.isEmpty())
            return Optional.empty();
        return Optional.ofNullable(references.get(0));
    }
}
