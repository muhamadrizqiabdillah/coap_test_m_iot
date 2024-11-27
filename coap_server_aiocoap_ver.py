import asyncio
from aiocoap import *
from aiocoap.resource import Resource, Site

class VideoResource(Resource):
    async def render_put(self, request):
        try:
            print("Received a PUT request.")
            print(f"Payload length: {len(request.payload)} bytes")
            
            # Check if the payload is indeed binary (raw image data)
            if isinstance(request.payload, bytes):
                # Define the filename to save the image
                filename = "received_image.jpg"
                
                # Write the binary data directly to a file
                with open(filename, "wb") as f:
                    f.write(request.payload)
                print(f"Image received and saved as {filename}")
                
                # Return a success response
                response = Message(code=CHANGED, payload=b"Image received")
            else:
                print("Error: Payload is not binary data.")
                response = Message(code=BAD_REQUEST, payload=b"Invalid payload format. Expected binary data.")
        except Exception as e:
            print(f"Error processing the image: {e}")
            response = Message(code=INTERNAL_SERVER_ERROR, payload=b"Internal Server Error")
        
        return response

async def main():
    root = Site()
    # Add the resource to handle the 'video' path
    root.add_resource(['video'], VideoResource())

    # Replace with your server's local IP address
    local_ip = "192.168.43.42"  # Replace with your actual local IP
    try:
        # Create and bind the CoAP server context to the specified IP and port 5683
        protocol = await Context.create_server_context(root, bind=(local_ip, 5683))
        print(f"CoAP server running on {local_ip}:5683")
    except Exception as e:
        print(f"Failed to bind server: {e}")
        return

    # Keep the server running indefinitely
    await asyncio.get_running_loop().create_future()

if __name__ == "__main__":
    asyncio.run(main())
