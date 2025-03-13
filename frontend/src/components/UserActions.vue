<template>
    <div>
        <p>User Actions</p>
        <button @click="leaveRoom()">Leave Room</button>
    </div>
</template>


<script setup>
    import { RouterLink, RouterView, useRouter } from 'vue-router'
    import { useApiStore } from '@/store/api'
    import { useUserStore } from '@/store/user'
    import axios from 'axios'


    const router = useRouter()
    const apiStore = useApiStore()
    const userStore = useUserStore()

    const room_id = userStore.current_room.room_id

    const leaveRoom = async () => {
        try {

            // TODO: Confirm before leaving
            const response = await axios.patch(`${apiStore.rest_url}/rooms/leave-room`, 
               {
                    room_id: room_id.value,
                    user_id: userStore.user_id,
               },
               {
                    headers: {
                        Authorization: `Bearer ${localStorage.getItem('token')}`
                    }
               }
            )

            if (response.status == 204) {
                userStore.current_room = null
                router.push('/home')
            }
        }
        catch (error) {
            console.error('Failed to leave room: ', error)
        }
    }
</script>


<style scoped>
button {
    font-size: 18pt;
    color: white;
    min-width: 70px;
    margin: auto;
    background-color: var(--foreground);
    border: var(--vue-green) solid 3px;
    text-align: center;
    align-items: center;
    text-decoration: none;
    cursor: pointer;
}

button:hover {
    color: var(--foreground);
    background-color: var(--vue-green)
}


</style>
